/*
 * Copyright (C) 2004-2009 Geometer Plus <contact@geometerplus.com>
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

#include <ZLInputStream.h>

#include "RtfDescriptionReader.h"
#include "../FormatPlugin.h"


RtfDescriptionReader::RtfDescriptionReader(DBBook &book) : RtfReader(book.encoding()), myBook(book) {
}

void RtfDescriptionReader::setEncoding(int code) {
	ZLEncodingCollection &collection = ZLEncodingCollection::instance();
	ZLEncodingConverterInfoPtr info = collection.info(code);
	if (!info.isNull()) {
		myConverter = info->createConverter();
		myBook.setEncoding(info->name());
	} else {
		myConverter = collection.defaultConverter();
	}
}

bool RtfDescriptionReader::readDocument(const std::string &fileName) {
	myDoRead = false;
	bool code = RtfReader::readDocument(fileName);
	if (myBook.authors().empty()) {
		myBook.authors().push_back( new DBAuthor() );
	}
	if (myBook.encoding().empty()) {
		myBook.setEncoding(PluginCollection::instance().DefaultEncodingOption.value());
	}
	return code;
}

void RtfDescriptionReader::addCharData(const char *data, size_t len, bool convert) {
	if (myDoRead) {
		if (convert) {
			myConverter->convert(myBuffer, data, data + len);
		} else {
			myBuffer.append(data, len);
		}
	}
}

void RtfDescriptionReader::switchDestination(DestinationType destination, bool on) {
	switch (destination) {
		case DESTINATION_INFO:
			if (!on) {
				interrupt();
			}
			break;
		case DESTINATION_TITLE:
			myDoRead = on;
			if (!on) {
				myBook.setTitle(myBuffer);
				myBuffer.erase();
			}
			break;
		case DESTINATION_AUTHOR:
			myDoRead = on;
			if (!on) {
				shared_ptr<DBAuthor> author = DBAuthor::create(myBuffer);
				if (!author.isNull()) {
					myBook.authors().push_back( author );
				}
				myBuffer.erase();
			}
			break;
		default:
			break;
	}
	if (!myBook.title().empty() && !myBook.authors().empty() && !myBook.encoding().empty()) {
		interrupt();
	}
}

void RtfDescriptionReader::insertImage(const std::string&, const std::string&, size_t, size_t) {
}

void RtfDescriptionReader::setFontProperty(FontProperty) {
}

void RtfDescriptionReader::newParagraph() {
}

void RtfDescriptionReader::setAlignment() {
}
