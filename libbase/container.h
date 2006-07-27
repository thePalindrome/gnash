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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(GNU_HASH_MAP)
# include <ext/hash_map>
#else
# ifdef WIN32_HASH_MAP
#  include <hash_map>
# endif
#endif

// FIXME: This ugly hack is for NetBSD, which seems to have a
// preprocessor problem, and won't define anything sensible like
// NETBSD we can use. Basically the problem is NetBSD has two thread
// implementations. One if the older pth library in /usr/pkg, and the
// other (the one we want to use) is /usr/pkg/phtread. Even with the
// corrent paths supplied, this one file barfs with GCC 3.3.3 on
// NetBSD, so screw it, and just hack it for now. We hope this entire
// file will be gond soon anyway.
#ifndef HAVE_WINSOCK_H
#define _LIB_PTHREAD_ 1
#define _LIB_PTHREAD_TYPES_H 1
#include <sys/types.h>
#include <pthread.h>
#include <time.h>

// This screws up MingW
#if 0
// And what's the point?
clock_t clock __P((void));
size_t strftime __P((char *, size_t, const char *, const struct tm *));
#endif // 0

#endif // ! HAVE_WINSOCK_H

//#include "tu_config.h"
#include "utility.h"
#include <stdlib.h>
#include <cstring>	// for strcmp and friends
//#include <new>	// for placement new
#include <vector>



template<class T>
class fixed_size_hash
// Computes a hash of an object's representation.
{
public:
	size_t	operator()(const T& data) const
	{
		const unsigned char*	p = (const unsigned char*) &data;
		int	size = sizeof(T);

		return sdbm_hash(p, size);
	}
};


template<class T>
class identity_hash
// Hash is just the input value; can use this for integer-indexed hash tables.
{
public:
	size_t	operator()(const T& data) const
	{
		return (size_t) data;
	}
};


//
// Thin wrappers around STL
//


//// @@@ crap compatibility crap
//#define StlAlloc(size) malloc(size)
//#define StlFree(ptr, size) free(ptr)

// Vitaly: hash from gameSWF. There are compiler problems with stdext:hash in Visual C
#if defined(_WIN32) || defined(WIN32)

template<class T, class U, class hash_functor = fixed_size_hash<T> >
class hash {
// Hash table, linear probing, internal chaining.  One
// interesting/nice thing about this implementation is that the table
// itself is a flat chunk of memory containing no pointers, only
// relative indices.  If the key and value types of the hash contain
// no pointers, then the hash can be serialized using raw IO.  Could
// come in handy.
//
// Never shrinks, unless you explicitly clear() it.  Expands on
// demand, though.  For best results, if you know roughly how big your
// table will be, default it to that size when you create it.
public:
	hash() : m_table(NULL) { }
	hash(int size_hint) : m_table(NULL) { set_capacity(size_hint); }
	~hash() { clear(); }

	// @@ need a "remove()"
//	char&	operator[](int index)
//	{
//	}

	U&	operator[](const T& key)
	{
		int	index = find_index(key);
		if (index >= 0)
		{
			return E(index).second;
		}
		add(key, (U) 0);
		index = find_index(key);
		if (index >= 0)
		{
			return E(index).second;
		}
		assert(0);

	}

	void	set(const T& key, const U& value)
	// Set a new or existing value under the key, to the value.
	{
		int	index = find_index(key);
		if (index >= 0)
		{
			E(index).second = value;
			return;
		}

		// Entry under key doesn't exist.
		add(key, value);
	}

	void	add(const T& key, const U& value)
	// Add a new value to the hash table, under the specified key.
	{
		assert(find_index(key) == -1);
//		if (find_index(key) != -1)
//		{
//			return;
//		}

		check_expand();
		assert(m_table);
		m_table->m_entry_count++;

		unsigned int	hash_value = hash_functor()(key);
		int	index = hash_value & m_table->m_size_mask;

		entry*	natural_entry = &(E(index));
		
		if (natural_entry->is_empty())
		{
			// Put the new entry in.
			new (natural_entry) entry(key, value, -1, hash_value);
		}
		else
		{
			// Find a blank spot.
			int	blank_index = index;
			for (;;)
			{
				blank_index = (blank_index + 1) & m_table->m_size_mask;
				if (E(blank_index).is_empty()) break;	// found it
			}
			entry*	blank_entry = &E(blank_index);

			if (int(natural_entry->m_hash_value & m_table->m_size_mask) == index)
			{
				// Collision.  Link into this chain.

				// Move existing list head.
				new (blank_entry) entry(*natural_entry);	// placement new, copy ctor

				// Put the new info in the natural entry.
				natural_entry->first = key;
				natural_entry->second = value;
				natural_entry->m_next_in_chain = blank_index;
				natural_entry->m_hash_value = hash_value;
			}
			else
			{
				// Existing entry does not naturally
				// belong in this slot.  Existing
				// entry must be moved.

				// Find natural location of collided element (i.e. root of chain)
				int	collided_index = natural_entry->m_hash_value & m_table->m_size_mask;
				for (;;)
				{
					entry*	e = &E(collided_index);
					if (e->m_next_in_chain == index)
					{
						// Here's where we need to splice.
						new (blank_entry) entry(*natural_entry);
						e->m_next_in_chain = blank_index;
						break;
					}
					collided_index = e->m_next_in_chain;
					assert(collided_index >= 0 && collided_index <= m_table->m_size_mask);
				}

				// Put the new data in the natural entry.
				natural_entry->first = key;
				natural_entry->second = value;
				natural_entry->m_hash_value = hash_value;
				natural_entry->m_next_in_chain = -1;
			}
		}
	}

	void	resize(int n)
	{
		clear();
	}

	void	clear()
	// Remove all entries from the hash table.
	{
		if (m_table)
		{
			// Delete the entries.
			for (int i = 0, n = m_table->m_size_mask; i <= n; i++)
			{
				entry*	e = &E(i);
				if (e->is_empty() == false)
				{
					e->clear();
				}
			}
			tu_free(m_table, sizeof(table) + sizeof(entry) * (m_table->m_size_mask + 1));
			m_table = NULL;
		}
	}

	bool	empty() const
	{
		return is_empty();
	}

	bool	is_empty() const
	// Returns true if the hash is empty.
	{
		return m_table == NULL || m_table->m_entry_count == 0;
	}


	bool	get(const T& key, U* value) const
	// Retrieve the value under the given key.
	//
	// If there's no value under the key, then return false and leave
	// *value alone.
	//
	// If there is a value, return true, and set *value to the entry's
	// value.
	//
	// If value == NULL, return true or false according to the
	// presence of the key, but don't touch *value.
	{
		int	index = find_index(key);
		if (index >= 0)
		{
			if (value) {
				*value = E(index).second;
			}
			return true;
		}
		return false;
	}

	int	size()
	{
		return m_table == NULL ? 0 : m_table->m_entry_count;
	}

	void	check_expand()
	// Resize the hash table to fit one more entry.  Often this
	// doesn't involve any action.
	{
		if (m_table == NULL) {
			// Initial creation of table.  Make a minimum-sized table.
			set_raw_capacity(16);
		} else if (m_table->m_entry_count * 3 > (m_table->m_size_mask + 1) * 2) {
			// Table is more than 2/3rds full.  Expand.
			set_raw_capacity(m_table->m_entry_count * 2);
		}
	}


	void	resize(size_t n)
	// Hint the bucket count to >= n.
	{
		// Not really sure what this means in relation to
		// STLport's hash_map... they say they "increase the
		// bucket count to at least n" -- but does that mean
		// their real capacity after resize(n) is more like
		// n*2 (since they do linked-list chaining within
		// buckets?).
		set_capacity(n);
	}

	void	set_capacity(int new_size)
	// Size the hash so that it can comfortably contain the given
	// number of elements.  If the hash already contains more
	// elements than new_size, then this may be a no-op.
	{
		int	new_raw_size = (new_size * 3) / 2;
		if (new_raw_size < size()) { return; }

		set_raw_capacity(new_raw_size);
	}

	// Behaves much like std::pair
	struct entry
	{
		int	m_next_in_chain;	// internal chaining for collisions
		size_t	m_hash_value;		// avoids recomputing.  Worthwhile?
		T	first;
		U	second;

		entry() : m_next_in_chain(-2) {}
		entry(const entry& e)
			: m_next_in_chain(e.m_next_in_chain), m_hash_value(e.m_hash_value), first(e.first), second(e.second)
		{
		}
		entry(const T& key, const U& value, int next_in_chain, int hash_value)
			: m_next_in_chain(next_in_chain), m_hash_value(hash_value), first(key), second(value)
		{
		}
		bool	is_empty() const { return m_next_in_chain == -2; }
		bool	is_end_of_chain() const { return m_next_in_chain == -1; }

		void	clear()
		{
			first.~T();	// placement delete
			second.~U();	// placement delete
			m_next_in_chain = -2;
		}
	};
	
	// Iterator API, like STL.

	struct const_iterator
	{
		T	get_key() const { return m_hash->E(m_index).first; }
		U	get_value() const { return m_hash->E(m_index).second; }

		const entry&	operator*() const
		{
			assert(is_end() == false && (m_hash->E(m_index).is_empty() == false));
			return m_hash->E(m_index);
		}
		const entry*	operator->() const { return &(operator*()); }

		void	operator++()
		{
			assert(m_hash);

			// Find next non-empty entry.
			if (m_index <= m_hash->m_table->m_size_mask)
			{
				m_index++;
				while (m_index <= m_hash->m_table->m_size_mask
				       && m_hash->E(m_index).is_empty())
				{
					m_index++;
				}
			}
		}

		bool	operator==(const const_iterator& it) const
		{
			if (is_end() && it.is_end())
			{
				return true;
			}
			else
			{
				return
					m_hash == it.m_hash
					&& m_index == it.m_index;
			}
		}

		bool	operator!=(const const_iterator& it) const { return ! (*this == it); }


		bool	is_end() const
		{
			return
				m_hash == NULL
				|| m_hash->m_table == NULL
				|| m_index > m_hash->m_table->m_size_mask;
		}

	protected:
		friend class hash<T,U,hash_functor>;

		const_iterator(const hash* h, int index)
			:
			m_hash(h),
			m_index(index)
		{
		}

		const hash*	m_hash;
		int	m_index;
	};
	friend struct const_iterator;

	// non-const iterator; get most of it from const_iterator.
	struct iterator : public const_iterator
	{
		// Allow non-const access to entries.
		entry&	operator*() const
		{
			assert(is_end() == false);
			return const_cast<hash*>(m_hash)->E(m_index);
		}
		entry*	operator->() const { return &(operator*()); }

	private:
		friend class hash<T,U,hash_functor>;

		iterator(hash* h, int i0)
			:
			const_iterator(h, i0)
		{
		}
	};
	friend struct iterator;


	iterator	begin()
	{
		if (m_table == 0) return iterator(NULL, 0);

		// Scan til we hit the first valid entry.
		int	i0 = 0;
		while (i0 <= m_table->m_size_mask
			&& E(i0).is_empty())
		{
			i0++;
		}
		return iterator(this, i0);
	}
	iterator	end() { return iterator(NULL, 0); }

	const_iterator	begin() const { return const_cast<hash*>(this)->begin(); }
	const_iterator	end() const { return const_cast<hash*>(this)->end(); }

	iterator	find(const T& key)
	{
		int	index = find_index(key);
		if (index >= 0)
		{
			return iterator(this, index);
		}
		return iterator(NULL, 0);
	}

	const_iterator	find(const T& key) const { return const_cast<hash*>(this)->find(key); }

private:
	void	operator=(const hash& h) { assert(0); }	// @@ TODO

	int	find_index(const T& key) const
	// Find the index of the matching entry.  If no match, then return -1.
	{
		if (m_table == NULL) return -1;

		size_t	hash_value = hash_functor()(key);
		int	index = hash_value & m_table->m_size_mask;

		const entry*	e = &E(index);
		if (e->is_empty()) return -1;
		if (int(e->m_hash_value & m_table->m_size_mask) != index) return -1;	// occupied by a collider

		for (;;)
		{
			assert((e->m_hash_value & m_table->m_size_mask) == (hash_value & m_table->m_size_mask));

			if (e->m_hash_value == hash_value && e->first == key)
			{
				// Found it.
				return index;
			}
			assert(! (e->first == key));	// keys are equal, but hash differs!

			// Keep looking through the chain.
			index = e->m_next_in_chain;
			if (index == -1) break;	// end of chain

			assert(index >= 0 && index <= m_table->m_size_mask);
			e = &E(index);

			assert(e->is_empty() == false);
		}
		return -1;
	}

	// Helpers.
	entry&	E(int index)
	{
		assert(m_table);
		assert(index >= 0 && index <= m_table->m_size_mask);
		return *(((entry*) (m_table + 1)) + index);
	}
	const entry&	E(int index) const
	{
		assert(m_table);
		assert(index >= 0 && index <= m_table->m_size_mask);
		return *(((entry*) (m_table + 1)) + index);
	}


	void	set_raw_capacity(int new_size)
	// Resize the hash table to the given size (Rehash the
	// contents of the current table).  The arg is the number of
	// hash table entries, not the number of elements we should
	// actually contain (which will be less than this).
	{
		if (new_size <= 0) {
			// Special case.
			clear();
			return;
		}

		// Force new_size to be a power of two.
		int	bits = fchop(log2((float)(new_size-1)) + 1);
		assert((1 << bits) >= new_size);

		new_size = 1 << bits;

		hash<T, U, hash_functor>	new_hash;
		new_hash.m_table = (table*) tu_malloc(sizeof(table) + sizeof(entry) * new_size);
		assert(new_hash.m_table);	// @@ need to throw (or something) on malloc failure!

		new_hash.m_table->m_entry_count = 0;
		new_hash.m_table->m_size_mask = new_size - 1;
		{for (int i = 0; i < new_size; i++)
		{
			new_hash.E(i).m_next_in_chain = -2;	// mark empty
		}}
		
		// Copy stuff to new_hash
		if (m_table)
		{
			for (int i = 0, n = m_table->m_size_mask; i <= n; i++)
			{
				entry*	e = &E(i);
				if (e->is_empty() == false)
				{
					// Insert old entry into new hash.
					new_hash.add(e->first, e->second);
					e->clear();	// placement delete of old element
				}
			}

			// Delete our old data buffer.
			tu_free(m_table, sizeof(table) + sizeof(entry) * (m_table->m_size_mask + 1));
		}

		// Steal new_hash's data.
		m_table = new_hash.m_table;
		new_hash.m_table = NULL;
	}

	struct table
	{
		int	m_entry_count;
		int	m_size_mask;
		// entry array goes here!
	};
	table*	m_table;
};	// WIN32 hash end

#else

template<class T, class U, class hash_functor = fixed_size_hash<T> >
class hash : public HASH_MAP_NS::hash_map<T, U, hash_functor >
{
public:
	typedef typename HASH_MAP_NS::hash_map<T, U, hash_functor>::const_iterator const_iterator;
	typedef typename HASH_MAP_NS::hash_map<T, U, hash_functor>::iterator iterator;

	// extra convenience interfaces
	void	add(const T& key, const U& value)
	{
		assert(find(key) == this->end());
		(*this)[key] = value;
	}

	bool	get(const T& key, U* value) const
	// Retrieve the value under the given key.
	//
	// If there's no value under the key, then return false and leave
	// *value alone.
	//
	// If there is a value, return true, and set *value to the entry's
	// value.
	//
	// If value == NULL, return true or false according to the
	// presence of the key, but don't touch *value.
	{
		const_iterator it = find(key);
		if (it != this->end())
		{
			if (value) *value = it->second;
			return true;
		}
		else
		{
			return false;
		}
	}
};
#endif	//GNUC

// // tu_string is a subset of std::string, for the most part
// class tu_string : public std::string
// {
// public:
// 	tu_string(const char* str) : std::string(str) {}
// 	tu_string() : std::string() {}
// 	tu_string(const tu_string& str) : std::string(str) {}

// 	int	length() const { return (int) std::string::length(); }
// };


// template<class U>
// class string_hash : public hash<tu_string, U, std::hash<std::string> >
// {
// };


//
// Homemade containers; almost strict subsets of STL.
//


#if defined(_WIN32) || defined(WIN32)
#pragma warning(disable : 4345)	// in MSVC 7.1, warning about placement new POD default initializer
#endif // _WIN32

class tu_stringi;

// String-like type.  Attempt to be memory-efficient with small strings.
class tu_string
{
public:
	tu_string() { m_local.m_size = 1; memset(m_local.m_buffer, 0, 15); }
	tu_string(const char* str)
	{
		m_local.m_size = 1;
		m_local.m_buffer[0] = 0;

		int	new_size = strlen(str);
		resize(new_size);
		strcpy(get_buffer(), str);
	}
	tu_string(const char* buf, int buflen)
	{
		m_local.m_size = 1;
		m_local.m_buffer[0] = 0;

		int	new_size = buflen;
		resize(new_size);
		memcpy(get_buffer(), buf, buflen);
		get_buffer()[buflen] = 0;	// terminate.
	}
	tu_string(const tu_string& str)
	{
		m_local.m_size = 1;
		m_local.m_buffer[0] = 0;

		resize(str.size());
		strcpy(get_buffer(), str.get_buffer());
	}
	tu_string(const uint32* wide_char_str)
	{
		m_local.m_size = 1;
		m_local.m_buffer[0] = 0;

		*this = wide_char_str;
	}
	tu_string(const uint16* wide_char_str)
	{
		m_local.m_size = 1;
		m_local.m_buffer[0] = 0;

		*this = wide_char_str;
	}

	~tu_string()
	{
		if (using_heap())
		{
			tu_free(m_heap.m_buffer, m_heap.m_capacity);
		}
	}

	operator const char*() const
	{
		return get_buffer();
	}

	const char*	c_str() const
	{
		return (const char*) (*this);
	}

	/// \brief
	/// If you need a const tu_stringi, don't create a new object;
	/// these things have the same internal representation.
	const tu_stringi&	to_tu_stringi() const { return *(const tu_stringi*) this; }

	/// \brief
	/// operator= returns void; if you want to know why, ask Charles Bloom :)
	/// (executive summary: a = b = c is an invitation to bad code)
	void	operator=(const char* str)
	{
		resize(strlen(str));
		strcpy(get_buffer(), str);
	}

	void	operator=(const tu_string& str)
	{
		resize(str.size());
		strcpy(get_buffer(), str.get_buffer());
	}

	bool	operator==(const char* str) const
	{
		return strcmp(*this, str) == 0;
	}

	bool	operator!=(const char* str) const
	{
		return strcmp(*this, str) != 0;
	}

	bool	operator==(const tu_string& str) const
	{
		return strcmp(*this, str) == 0;
	}

	bool	operator!=(const tu_string& str) const
	{
		return strcmp(*this, str) != 0;
	}

	int	length() const
	{
		if (using_heap() == false)
		{
			return m_local.m_size - 1;
		}
		else
		{
			return m_heap.m_size - 1;
		}
	}
	int	size() const { return length(); }

	char&	operator[](int index)
	{
		assert(index >= 0 && index <= size());
		return get_buffer()[index];
	}
	const char&	operator[](int index) const
	{
		return (*(const_cast<tu_string*>(this)))[index];
	}

	void	operator+=(const char* str)
	{
		int	str_length = strlen(str);
		int	old_length = length();
		assert(old_length >= 0);
		resize(old_length + str_length);
		strcpy(get_buffer() + old_length, str);
	}

	void	operator+=(char ch)
	{
		int	old_length = length();
		assert(old_length >= 0);
		resize(old_length + 1);
		strncpy(get_buffer() + old_length, (char *)&ch, 1);
	}

	// Append wide char.  Both versions of wide char.
	void	append_wide_char(uint16 ch);
	void	append_wide_char(uint32 ch);

	void	operator+=(const tu_string& str)
	{
		int	str_length = str.length();
		int	old_length = length();
		assert(old_length >= 0);
		resize(old_length + str_length);
		strcpy(get_buffer() + old_length, str.c_str());
	}

	tu_string	operator+(const char* str) const
	// NOT EFFICIENT!  But convenient.
	{
		tu_string	new_string(*this);
		new_string += str;
		return new_string;
	}

	bool	operator<(const char* str) const
	{
		return strcmp(c_str(), str) < 0;
	}
	bool	operator<(const tu_string& str) const
	{
		return *this < str.c_str();
	}
	bool	operator>(const char* str) const
	{
		return strcmp(c_str(), str) > 0;
	}
	bool	operator>(const tu_string& str) const
	{
		return *this > str.c_str();
	}

	void clear()
	{
		resize(0);
	}
	
	// Sets buffer size to new_size+1 (i.e. enough room for
	// new_size chars, plus terminating 0).
	void	resize(int new_size);

	// Set *result to the UTF-8 encoded version of wstr[].
	// Both version of wchar_t.
	//
	// Could add operator= overloads, but maybe it's better to
	// keep this very explicit.
	static void	encode_utf8_from_wchar(tu_string* result, const uint32* wstr);
	static void	encode_utf8_from_wchar(tu_string* result, const uint16* wstr);

	// Utility: case-insensitive string compare.  stricmp() is not
	// ANSI or POSIX, doesn't seem to appear in Linux.
	static int	stricmp(const char* a, const char* b);

	// Return the Unicode char at the specified character
	// position.  index is in UTF-8 chars, NOT bytes.
	uint32	utf8_char_at(int index) const;

	// Return the string in this container as all upper case letters
	tu_string utf8_to_upper() const;

	// Return the string in this container as all lower case letters
	tu_string utf8_to_lower() const;

	// Return the number of UTF-8 characters in the given
	// substring buffer.  You must pass in a valid buffer length;
	// this routine does not look for a terminating \0.
	static int	utf8_char_count(const char* buf, int buflen);

	int	utf8_length() const { return utf8_char_count(get_buffer(), length()); }

	// Returns a tu_string that's a substring of this.  start and
	// end are in UTF-8 character positions (NOT bytes).
	//
	// start is the index of the first character you want to include.
	//
	// end is the index one past the last character you want to include.
	tu_string	utf8_substring(int start, int end) const;

private:
	char*	get_buffer()
	{
		if (using_heap() == false)
		{
			return m_local.m_buffer;
		}
		else
		{
			return m_heap.m_buffer;
		}
	}

	const char*	get_buffer() const
	{
		return const_cast<tu_string*>(this)->get_buffer();
	}


	bool	using_heap() const
	{
		bool	heap = (m_heap.m_all_ones == (char) ~0);
		return heap;
	}

	// The idea here is that tu_string is a 16-byte structure,
	// which uses internal storage for strings of 14 characters or
	// less.  For longer strings, it allocates a heap buffer, and
	// keeps the buffer-tracking info in the same bytes that would
	// be used for internal string storage.
	//
	// A string that's implemented like vector<char> is typically
	// 12 bytes plus heap storage, so this seems like a decent
	// thing to try.  Also, a zero-length string still needs a
	// terminator character, which with vector<char> means an
	// unfortunate heap alloc just to hold a single '0'.
	union
	{
		// Internal storage.
		struct
		{
			char	m_size;
			char	m_buffer[15];
		} m_local;

		// Heap storage.
		struct
		{
			char	m_all_ones;	// flag to indicate heap storage is in effect.
			int	m_size;
			int	m_capacity;
			char*	m_buffer;
		} m_heap;
	};
};


// String-like type; comparisons are CASE INSENSITIVE.
// Uses tu_string for implementation.
class tu_stringi
{
public:
	tu_stringi() {}
	tu_stringi(const char* str) : m_string(str) {}
	tu_stringi(const tu_string& str) : m_string(str) {}
	tu_stringi(const tu_stringi& stri) : m_string(stri.c_str()) {}

	~tu_stringi() {}

	operator const char*() const { return (const char*) m_string; }
	const char*	c_str() const { return m_string.c_str(); }
	void	operator=(const char* str) { m_string = str; }
	void	operator=(const tu_string& str) { m_string = str; }
	void	operator=(const tu_stringi& str) { m_string = str.m_string; }
	int	length() const { return m_string.length(); }
	int	size() const { return length(); }
	char&	operator[](int index) { return m_string[index]; }
	const char&	operator[](int index) const { return m_string[index]; }
	void	operator+=(const char* str) { m_string += str; }
	void	operator+=(const tu_string& str) { m_string += str; }
	void	operator+=(const tu_stringi& str) { m_string += str.m_string; }
	tu_stringi	operator+(const char* str) const { return tu_stringi(m_string + str); }

	// The special stuff.
	tu_string& to_tu_string() { return m_string; }
	const tu_string& to_tu_string() const { return m_string; }

	bool	operator==(const char* str) const
	{
		return tu_string::stricmp(*this, str) == 0;
	}
	bool	operator==(const tu_stringi& str) const
	{
		return tu_string::stricmp(*this, str) == 0;
	}
	bool	operator<(const char* str) const
	{
		return tu_string::stricmp(c_str(), str) < 0;
	}
	bool	operator<(const tu_stringi& str) const
	{
		return *this < str.c_str();
	}
	bool	operator>(const char* str) const
	{
		return tu_string::stricmp(c_str(), str) > 0;
	}
	bool	operator>(const tu_stringi& str) const
	{
		return *this > str.c_str();
	}

	void	resize(int new_size) { m_string.resize(new_size); }

private:
	tu_string	m_string;
};


template<class T>
class string_hash_functor
// Computes a hash of a string-like object (something that has
// ::length() and ::[int]).
{
public:
	size_t	operator()(const T& data) const
	{
		int	size = data.length();

		return bernstein_hash((const char*) data, size);
	}
};


template<class U>
class string_hash : public hash<tu_string, U, string_hash_functor<tu_string> >
{
};


template<class T>
class stringi_hash_functor
// Computes a case-insensitive hash of a string-like object (something that has
// ::length() and ::[int] and tolower(::[])).
{
public:
	size_t	operator()(const T& data) const
	{
		int	size = data.length();

		return bernstein_hash_case_insensitive((const char*) data, size);
	}
};


// Case-insensitive string hash.
template<class U>
class stringi_hash : public hash<tu_stringi, U, stringi_hash_functor<tu_stringi> >
{
};


// Utility: handy sprintf wrapper.
tu_string string_printf(const char* fmt, ...)
#ifdef __GNUC__
	// use the following to catch errors: (only with gcc)
	__attribute__((format (printf, 1, 2)))
#endif	// not __GNUC__
;


#endif // __CONTAINER_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
