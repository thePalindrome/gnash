// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//
//

/* $Id: NetStream.cpp,v 1.24 2007/01/30 10:52:15 strk Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "NetStream.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "GnashException.h"

#include "movie_root.h"

namespace gnash {
 
static void netstream_new(const fn_call& fn);
static void netstream_close(const fn_call& fn);
static void netstream_pause(const fn_call& fn);
static void netstream_play(const fn_call& fn);
static void netstream_seek(const fn_call& fn);
static void netstream_setbuffertime(const fn_call& fn);
static void netstream_time(const fn_call& fn);
static as_object* getNetStreamInterface();

netstream_as_object::netstream_as_object()
	:
	as_object(getNetStreamInterface())
{
	obj.set_parent(this);
}

netstream_as_object::~netstream_as_object()
{
}

static void
netstream_new(const fn_call& fn)
{

	netstream_as_object *netstream_obj = new netstream_as_object;

	if (fn.nargs > 0)
	{
		as_object* nc = fn.arg(0).to_object();
		if ( nc ) netstream_obj->obj.setNetCon(nc);
		else
		{
			IF_VERBOSE_ASCODING_ERRORS(
				log_aserror("First argument "
					"to NetConnection constructor "
					"doesn't cast to an Object (%s)",
					fn.arg(0).to_string());
			);
		}
	}
	fn.result->set_as_object(netstream_obj);

}

// Wrapper around dynamic_cast to implement user warning.
// To be used by builtin properties and methods.
static netstream_as_object*
ensure_netstream(as_object* obj)
{
	netstream_as_object* ret = dynamic_cast<netstream_as_object*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for NetStream objects called against non-NetStream instance");
	}
	return ret;
}


static void netstream_close(const fn_call& fn)
{
	netstream_as_object* ns = ensure_netstream(fn.this_ptr);
	ns->obj.close();
}

static void netstream_pause(const fn_call& fn)
{
	netstream_as_object* ns = ensure_netstream(fn.this_ptr);
	
	// mode: -1 ==> toogle, 0==> pause, 1==> play
	int mode = -1;
	if (fn.nargs > 0)
	{
		mode = fn.arg(0).to_bool() ? 0 : 1;
	}
	ns->obj.pause(mode);	// toggle mode
}

static void netstream_play(const fn_call& fn)
{
	netstream_as_object* ns = ensure_netstream(fn.this_ptr);

	if (fn.nargs < 1)
	{
		IF_VERBOSE_ASCODING_ERRORS(
		log_aserror("NetStream play needs args");
		);
		return;
	}

	if (ns->obj.play(fn.arg(0).to_string()) != 0)
	{
		ns->obj.close();
	};
}

static void netstream_seek(const fn_call& fn) {
	netstream_as_object* ns = ensure_netstream(fn.this_ptr);

	double time = 0;
	if (fn.nargs > 0)
	{
		time = fn.arg(0).to_number();
	}
	ns->obj.seek(time);

}
static void netstream_setbuffertime(const fn_call& fn) {
	netstream_as_object* ns = ensure_netstream(fn.this_ptr);
	UNUSED(ns);
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

// Both a getter and a (do-nothing) setter for time
static void
netstream_time(const fn_call& fn)
{

	netstream_as_object* ns = ensure_netstream(fn.this_ptr);

	if ( fn.nargs == 0 )
	{
		fn.result->set_double(ns->obj.time());
	}
	else
	{
		IF_VERBOSE_ASCODING_ERRORS(
			log_aserror("Tried to set read-only property NetStream.time");
		);
	}
}

void
attachNetStreamInterface(as_object& o)
{

	o.init_member("close", &netstream_close);
	o.init_member("pause", &netstream_pause);
	o.init_member("play", &netstream_play);
	o.init_member("seek", &netstream_seek);
	o.init_member("setBufferTime", &netstream_setbuffertime);


    // Properties

    boost::intrusive_ptr<builtin_function> gettersetter;

    gettersetter = new builtin_function(&netstream_time, NULL);
    o.init_property("time", *gettersetter, *gettersetter);

}

static as_object*
getNetStreamInterface()
{

	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
		attachNetStreamInterface(*o);
	}

	return o.get();
}

// extern (used by Global.cpp)
void netstream_class_init(as_object& global)
{

	// This is going to be the global NetStream "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&netstream_new, getNetStreamInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachNetStreamInterface(*cl);
		     
	}

	// Register _global.String
	global.init_member("NetStream", cl.get());

}
} // end of gnash namespace

