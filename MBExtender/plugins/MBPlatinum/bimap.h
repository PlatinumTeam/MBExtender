//-----------------------------------------------------------------------------
// Copyright (c) 2016 The Platinum Team
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//-----------------------------------------------------------------------------

// bimap.h
// Totally not written for CS1200

// #ifndef CS1200_BIMAP_H
// #define CS1200_BIMAP_H
#pragma once

template<typename Key, typename Value, template<typename ...> class structure>
class bimap {
private:
	structure<Key, Value> key_map;
	structure<Value, Key> value_map;
public:
	template<typename K, typename V>
	class iterator {
	private:
		friend class bimap<K, V, structure>;
		friend class bimap<V, K, structure>;

		iterator(typename structure<K, V>::iterator rep) : rep(rep) {}

		typename structure<K, V>::iterator rep;
	public:
		iterator() : rep(NULL) {}
		iterator(const iterator &other) : rep(other.rep) {}

//		iterator<V, K> follow_link() const {
//
//		}
		bool operator==(const iterator &other) const {
			return rep == other.rep;
		}
		bool operator!=(const iterator &other) const {
			return !operator==(other);
		}
		std::pair<K, V> operator*() const {
			return *rep;
		}
		std::pair<K, V> *operator->() const {
			return (std::pair<K, V>*)(&*rep);
		}

		iterator &operator++() {
			++rep;
			return *this;
		}
		iterator &operator--() {
			--rep;
			return *this;
		}

		iterator operator++(int) {
			iterator temp(*this);
			++(*this);
			return temp;
		}
		iterator operator--(int) {
			iterator temp(*this);
			--(*this);
			return temp;
		}
	};
	typedef iterator<Key, Value> key_iterator;
	typedef iterator<Value, Key> value_iterator;

	decltype(key_map.size()) size() const {
		return key_map.size();
	}

	std::pair<key_iterator, bool> insert(const std::pair<Key, Value> &pair) {
		auto inserted = key_map.insert(pair);
		if (inserted.second) {
			//Insert into other
			auto value_insert = value_map.insert(std::make_pair(pair.second, pair.first));

			if (!value_insert.second) {
				key_map.erase(pair.first);

				return std::make_pair(key_iterator(find(value_insert.first->second)), false);
			}
		}
		return std::make_pair(key_iterator(inserted.first), inserted.second);
	}
	int erase(const Key &key) {
		auto found = key_map.find(key);
		if (found != key_map.end()) {
			value_map.erase(found->second);
		}
		return key_map.erase(key);
	}
	int erase(const Value &value) {
		auto found = value_map.find(value);
		if (found != value_map.end()) {
			key_map.erase(found->second);
		}
		return value_map.erase(value);
	}

	key_iterator find(const Key &key) {
		return key_iterator(key_map.find(key));
	}
	value_iterator find(const Value &value) {
		return value_iterator(value_map.find(value));
	}

	key_iterator key_begin() {
		return key_iterator(key_map.begin());
	}
	key_iterator key_end() {
		return key_iterator(key_map.end());
	}
	value_iterator value_begin() {
		return value_iterator(value_map.begin());
	}
	value_iterator value_end() {
		return value_iterator(value_map.end());
	}

	const Key &operator[](const Value &value) {
		return find(value)->second;
	}
	const Value &operator[](const Key &key) {
		return find(key)->second;
	}
};
