// tag_loaders.cpp: SWF tags loaders, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h" // HAVE_ZLIB_H, USE_SWFTREE
#endif

#include "tag_loaders.h"

#include <limits>
#include <cassert>
#include <boost/static_assert.hpp>

#include "IOChannel.h"
#include "utility.h"
#include "log.h"
#include "SWFStream.h"
#include "zlib_adapter.h"
#include "SWFMovieDefinition.h"
#include "SWF.h"
#include "swf/TagLoadersTable.h"
#include "GnashException.h"
#include "RunResources.h"
#include "Renderer.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "GnashImageJpeg.h"

#ifdef HAVE_ZLIB_H
#include <zlib.h>
#endif

namespace gnash {
namespace SWF {

/// Anonymous namespace
namespace {

/// Provide an IOChannel interface around a SWFStream for reading 
/// embedded image data
class StreamAdapter : public IOChannel
{
    SWFStream& s;
    std::streampos startPos;
    std::streampos endPos;
    std::streampos currPos;

    StreamAdapter(SWFStream& str, std::streampos maxPos)
        :
        s(str),
        startPos(s.tell()),
        endPos(maxPos),
        currPos(startPos)
    {
        assert(endPos > startPos);
    }

    virtual ~StreamAdapter() {}

    virtual std::streamsize read(void* dst, std::streamsize bytes) {
        std::streamsize bytesLeft = endPos - currPos;
        if (bytesLeft < bytes) {
            if (!bytesLeft) return 0;
            //log_debug("Requested to read past end of stream range");
            bytes = bytesLeft;
        }
        std::streamsize actuallyRead = s.read(static_cast<char*>(dst), bytes);
        currPos += actuallyRead;
        return actuallyRead;
    }

    virtual void go_to_end() {
        s.seek(endPos);
    }

    virtual bool eof() const {
        return (currPos == endPos);
    }

    // Return false on failure, true on success
    virtual bool seek(std::streampos pos) {
        // SWFStream::seek() returns true on success
        if (s.seek(pos)) {
            currPos = pos;
            return true;
        }
        return false;
    }

    virtual size_t size() const {
        return (endPos - startPos);
    }

    virtual std::streampos tell() const {
        return currPos;
    }
    
    virtual bool bad() const {
        // Is there any point in this?
        return false;
    }

public:

    /// Get an IOChannel from a gnash::SWFStream
    static std::auto_ptr<IOChannel> getFile(SWFStream& str,
            unsigned long endPos) {
        std::auto_ptr<IOChannel> ret (new StreamAdapter(str, endPos));
        return ret;
    }
};

} // anonymous namespace

// Load JPEG compression tables that can be used to load
// images further along in the SWFStream.
void
jpeg_tables_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& /*r*/)
{
    assert(tag == SWF::JPEGTABLES);

    IF_VERBOSE_PARSE(
        log_parse(_("  jpeg_tables_loader"));
    );

    const std::streampos currPos = in.tell();
    const std::streampos endPos = in.get_tag_end_position();

    assert(endPos >= currPos);

    const unsigned long jpegHeaderSize = endPos - currPos;

    if (!jpegHeaderSize) {
        log_debug(_("No bytes to read in JPEGTABLES tag at offset %d"),
                currPos);
    }

    std::auto_ptr<image::JpegInput> input;

    try {
        // NOTE: we cannot limit input SWFStream here as the same jpeg::input
        // instance will be used for reading subsequent DEFINEBITS and similar
        // tags, which are *different* tags, so have different boundaries !!
        //
        // Anyway the actual reads are limited to currently opened tag as 
        // of gnash::SWFStream::read(), so this is not a problem.
        //
        boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                    std::numeric_limits<std::streamsize>::max()).release());
        //  transfer ownership to the image::JpegInput
        input = image::JpegInput::createSWFJpeg2HeaderOnly(ad, jpegHeaderSize);

    }
    catch (const std::exception& e) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Error creating header-only jpeg2 input: %s",
                e.what());
        );
        return;
    }

    log_debug("Setting jpeg loader to %p", (void*)input.get());
    m.set_jpeg_loader(input);
}

// A JPEG image without included tables; those should be in an
// existing image::JpegInput object stored in the movie.
void
define_bits_jpeg_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINEBITS); // 6

    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    if (m.getBitmap(id)) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBITS: Duplicate id (%d) for bitmap "
                    "DisplayObject - discarding it"), id);
        );
        return;
    }

    // Read the image data.
    image::JpegInput* j_in = m.get_jpeg_loader();
    if (!j_in) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBITS: No jpeg loader registered in movie "
                    "definition - discarding bitmap DisplayObject %d"), id);
        );
        return;
    }

    j_in->discardPartialBuffer();
    
    std::auto_ptr<image::GnashImage> im;
    try {
        im = image::JpegInput::readSWFJpeg2WithTables(*j_in);
    }
    catch (const std::exception& e) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror("Error reading jpeg2 with headers for DisplayObject "
                "id %d: %s", id, e.what());
        );
        return;
    }
    
    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(im);

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);
}


void
define_bits_jpeg2_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINEBITSJPEG2); // 21

    in.ensureBytes(2);
    boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  define_bits_jpeg2_loader: charid = %d pos = %ld"),
              id, in.tell());
    );
    
    if (m.getBitmap(id)) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBITSJPEG2: Duplicate id (%d) for bitmap "
                    "DisplayObject - discarding it"), id);
        );
        return;
    }

    char buf[3];
    if (in.read(buf, 3) < 3) {
        log_swferror(_("DEFINEBITS data too short to read type header"));
        return;
    }
    in.seek(in.tell() - 3);

    FileType ft = GNASH_FILETYPE_JPEG;  

    // Check the data type. The pp version 9,0,115,0 supports PNG and GIF
    // in DefineBits tags, though it is not documented. The version makes
    // no difference.
    if (std::equal(buf, buf + 3, "\x89PN")) {
        ft = GNASH_FILETYPE_PNG;
    }
    else if (std::equal(buf, buf + 3, "GIF")) {
        ft = GNASH_FILETYPE_GIF;
    }

    // Read the image data.
    boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                in.get_tag_end_position()).release());

    std::auto_ptr<image::GnashImage> im(image::Input::readImageData(ad, ft));

    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(im);

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);

}

#ifdef HAVE_ZLIB_H
void inflate_wrapper(SWFStream& in, void* buffer, int buffer_bytes)
    // Wrapper function -- uses Zlib to uncompress in_bytes worth
    // of data from the input file into buffer_bytes worth of data
    // into *buffer.
{
    assert(buffer);
    assert(buffer_bytes > 0);

    z_stream d_stream; /* decompression SWFStream */

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = 0;
    d_stream.avail_in = 0;

    d_stream.next_out = static_cast<Byte*>(buffer);
    d_stream.avail_out = static_cast<uInt>(buffer_bytes);

    int err = inflateInit(&d_stream);
    if (err != Z_OK) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("inflate_wrapper() inflateInit() returned %d (%s)"),
                err, d_stream.msg);
        );
        return;
    }

    const size_t CHUNKSIZE = 256;

    boost::uint8_t buf[CHUNKSIZE];
    unsigned long endTagPos = in.get_tag_end_position();

    for (;;) {
        unsigned int chunkSize = CHUNKSIZE;
        assert(in.tell() <= endTagPos);
        unsigned int availableBytes =  endTagPos - in.tell();
        if (availableBytes < chunkSize) {
            if (!availableBytes) {
                // nothing more to read
                IF_VERBOSE_MALFORMED_SWF(
                    log_swferror(_("inflate_wrapper(): no end of zstream "
                        "found within swf tag boundaries"));
                );
                break;
            }
            chunkSize = availableBytes;
        }
    
        BOOST_STATIC_ASSERT(sizeof(char) == sizeof(boost::uint8_t));

        // Fill the buffer    
        in.read(reinterpret_cast<char*>(buf), chunkSize);
        d_stream.next_in = &buf[0];
        d_stream.avail_in = chunkSize;

        err = inflate(&d_stream, Z_SYNC_FLUSH);
        if (err == Z_STREAM_END) {
            // correct end
            break;
        }

        if (err != Z_OK) {
            IF_VERBOSE_MALFORMED_SWF(
                log_swferror(_("inflate_wrapper() inflate() returned %d (%s)"),
                    err, d_stream.msg);
            );
            break;
        }
    }

    err = inflateEnd(&d_stream);
    if (err != Z_OK) {
        log_error(_("inflate_wrapper() inflateEnd() return %d (%s)"),
                err, d_stream.msg);
    }
}
#endif // HAVE_ZLIB_H


// loads a define_bits_jpeg3 tag. This is a jpeg file with an alpha
// channel using zlib compression.
void
define_bits_jpeg3_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINEBITSJPEG3); // 35

    in.ensureBytes(2);
    const boost::uint16_t id = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  define_bits_jpeg3_loader: charid = %d pos = %lx"),
              id, in.tell());
    );

    in.ensureBytes(4);
    const boost::uint32_t jpeg_size = in.read_u32();
    const boost::uint32_t alpha_position = in.tell() + jpeg_size;

#ifndef HAVE_ZLIB_H
    log_error(_("gnash is not linked to zlib -- can't load jpeg3 image data"));
    return;
#else

    // Read rgb data.
    boost::shared_ptr<IOChannel> ad(StreamAdapter::getFile(in,
                alpha_position).release());
    std::auto_ptr<image::ImageRGBA> im = image::Input::readSWFJpeg3(ad);
    
    /// Failure to read the jpeg.
    if (!im.get()) return;

    // Read alpha channel.
    in.seek(alpha_position);

    const size_t imWidth = im->width();
    const size_t imHeight = im->height();
    const size_t bufferLength = imWidth * imHeight;

    boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[bufferLength]);

    inflate_wrapper(in, buffer.get(), bufferLength);

    // TESTING:
    // magical trevor contains this tag
    //  ea8bbad50ccbc52dd734dfc93a7f06a7  6964trev3c.swf
    image::mergeAlpha(*im, buffer.get(), bufferLength);

    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(
            static_cast<std::auto_ptr<image::GnashImage> >(im));

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);
#endif
}


void
define_bits_lossless_2_loader(SWFStream& in, TagType tag, movie_definition& m,
		const RunResources& r)
{
    assert(tag == SWF::DEFINELOSSLESS || tag == SWF::DEFINELOSSLESS2);
    in.ensureBytes(2 + 2 + 2 + 1); // the initial header 

    const boost::uint16_t id = in.read_u16();

    // 3 == 8 bit, 4 == 16 bit, 5 == 32 bit
    const boost::uint8_t bitmap_format = in.read_u8();
    const boost::uint16_t width = in.read_u16();
    const boost::uint16_t height = in.read_u16();

    IF_VERBOSE_PARSE(
        log_parse(_("  defbitslossless2: tag = %d, id = %d, fmt = %d, "
                "w = %d, h = %d"), tag, id, bitmap_format, width, height);
    );

    if (!width || !height) {
         IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("Bitmap DisplayObject %d has a height or width of 0"),
                id);
        );   
        return;  
    }

    // No need to parse any further if it already exists, as we aren't going
    // to add it.
    if (m.getBitmap(id)) {
        IF_VERBOSE_MALFORMED_SWF(
            log_swferror(_("DEFINEBITSLOSSLESS: Duplicate id (%d) "
                           "for bitmap DisplayObject - discarding it"), id);
        );
    }

#ifndef HAVE_ZLIB_H
    log_error(_("gnash is not linked to zlib -- can't load zipped image data"));
    return;
#else

    unsigned short channels;
    std::auto_ptr<image::GnashImage> image;
    bool alpha = false;

    try {
        switch (tag) {
            case SWF::DEFINELOSSLESS:
                image.reset(new image::ImageRGB(width, height));
                channels = 3;
                break;
            case SWF::DEFINELOSSLESS2:
                image.reset(new image::ImageRGBA(width, height));
                channels = 4;
                alpha = true;
                break;
            default:
                // This is already asserted.
                std::abort();
        }
    }
    catch (const std::bad_alloc&) {
        // Image constructors will throw bad_alloc if they don't like the
        // size. This isn't usually from operator new.
        log_error(_("Will not allocate %1%x%2% image in DefineBitsLossless "
                "tag"), width, height);
        return;
    }

    unsigned short bytes_per_pixel;
    int colorTableSize = 0;

    switch (bitmap_format) {
        case 3:
            bytes_per_pixel = 1;
            in.ensureBytes(1);
            // SWF stores one less than the actual size.
            colorTableSize = in.read_u8() + 1;
            break;

        case 4:
            bytes_per_pixel = 2;
            break;

        case 5:
            bytes_per_pixel = 4;
            break;

        default:
            log_error(_("Unknown bitmap format. Ignoring"));
            return;
    }

    const size_t pitch = (width * bytes_per_pixel + 3) &~ 3;
    const size_t bufSize = colorTableSize * channels + pitch * height;
    boost::scoped_array<boost::uint8_t> buffer(new boost::uint8_t[bufSize]);

    inflate_wrapper(in, buffer.get(), bufSize);
    assert(in.tell() <= in.get_tag_end_position());

    switch (bitmap_format) {

        case 3:
        {
            // 8-bit data, preceded by a palette.
            boost::uint8_t* colorTable = buffer.get();

            for (size_t j = 0; j < height; ++j) {
                boost::uint8_t* inRow = buffer.get() + 
                    colorTableSize * channels + j * pitch;

                boost::uint8_t* outRow = scanline(*image, j);
                for (size_t i = 0; i < width; ++i) {
                    boost::uint8_t pixel = inRow[i * bytes_per_pixel];
                    outRow[i * channels + 0] = colorTable[pixel * channels + 0];
                    outRow[i * channels + 1] = colorTable[pixel * channels + 1];
                    outRow[i * channels + 2] = colorTable[pixel * channels + 2];
                    if (alpha) {
                        outRow[i * channels + 3] =
                            colorTable[pixel * channels + 3];
                    }
                }
            }
            break;
        }

        case 4:
            // 16 bits / pixel

            for (size_t j = 0; j < height; ++j) {

                boost::uint8_t* inRow = buffer.get() + j * pitch;
                boost::uint8_t* outRow = scanline(*image, j);
                for (size_t i = 0; i < width; ++i) {
                    const boost::uint16_t pixel = inRow[i * 2] |
                        (inRow[i * 2 + 1] << 8);

                    // How is the data packed??? Whoever wrote this was
                    // just guessing here that it's 565!
                    outRow[i * channels + 0] = (pixel >> 8) & 0xF8;    // red
                    outRow[i * channels + 1] = (pixel >> 3) & 0xFC;    // green
                    outRow[i * channels + 2] = (pixel << 3) & 0xF8;    // blue
 
                    // This was saved to the first byte before, but that
                    // can hardly be correct.
                    // Real examples of this format are rare to non-existent.
                    if (alpha) {
                        outRow[i * channels + 3] = 255;
                    }
                }
            }
            break;

        case 5:
            // Need to re-arrange ARGB into RGB or RGBA.
            for (size_t j = 0; j < height; ++j) {
                boost::uint8_t* inRow = buffer.get() + j * pitch;
                boost::uint8_t* outRow = scanline(*image, j);
                const int inChannels = 4;

                for (size_t i = 0; i < width; ++i) {
                    // Copy pixels 1-3.
                    std::copy(&inRow[i * inChannels + 1],
                            &inRow[i * inChannels + 4], &outRow[i * channels]);

                    // Add the alpha channel if necessary.
                    if (alpha) {
                        outRow[i * channels + 3] = inRow[i * 4];
                    }
                }
            }
            break;

    }

    Renderer* renderer = r.renderer();
    if (!renderer) {
        IF_VERBOSE_PARSE(log_parse(_("No renderer, not adding bitmap")));
        return;
    }    
    boost::intrusive_ptr<CachedBitmap> bi = renderer->createCachedBitmap(image);

    // add bitmap to movie under DisplayObject id.
    m.addBitmap(id, bi);
#endif // HAVE_ZLIB_H

}

} // namespace gnash::SWF
} // namespace gnash

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
