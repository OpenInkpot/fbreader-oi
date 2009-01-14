/*
 * Copyright (C) 2008-2009 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "MergedStream.h"

bool MergedStream::open() {
	close();
	resetToStart();
	myOffset = 0;
	myCurrentStream = nextStream();
	return !myCurrentStream.isNull() && myCurrentStream->open();
}

size_t MergedStream::read(char *buffer, size_t maxSize) {
	size_t bytesToRead = maxSize;
	while ((bytesToRead > 0) && !myCurrentStream.isNull()) {
		size_t len = myCurrentStream->read(buffer, bytesToRead);
		bytesToRead -= len;
		if (buffer != 0) {
			buffer += len;
		}
		if (bytesToRead != 0) {
			*buffer++ = '\n';
			bytesToRead--;
			myCurrentStream = nextStream();
			if (myCurrentStream.isNull() || !myCurrentStream->open()) {
				break;
			}
		}
	}
	myOffset += maxSize - bytesToRead;
	return maxSize - bytesToRead;
}

void MergedStream::close() {
	myCurrentStream.reset();
}

void MergedStream::seek(int offset, bool absoluteOffset) {
	// works for nonnegative offsets only
	if (absoluteOffset) {
		offset -= myOffset;
	}
	read(0, offset);
}

size_t MergedStream::offset() const {
	return myOffset;
}

size_t MergedStream::sizeOfOpened() {
	// coudn't be implemented
	return 0;
}
