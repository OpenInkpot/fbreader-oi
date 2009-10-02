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

#ifndef __BOOKINFODIALOG_H__
#define __BOOKINFODIALOG_H__

#include <string>

#include <ZLOptionEntry.h>

#include "../collection/BookCollection.h"
#include "../formats/FormatPlugin.h"

#include "../database/booksdb/DBAuthor.h"

class ZLOptionsDialog;
class AuthorDisplayNameEntry;
class SeriesTitleEntry;
class BookNumberEntry;
class BookTagEntry;


class BookInfoDialog {

public:
	BookInfoDialog(const BookCollection &collection, shared_ptr<DBBook> book);

	ZLOptionsDialog &dialog();

private:
	void initAuthorEntries();
	void initTagEntries();

private:
	shared_ptr<ZLOptionsDialog> myDialog;
	const BookCollection &myCollection;
	shared_ptr<DBBook> myBook;
	shared_ptr<FormatInfoPage> myFormatInfoPage;

	ZLComboOptionEntry *myEncodingSetEntry;
	ZLComboOptionEntry *myEncodingEntry;
	ZLComboOptionEntry *myLanguageEntry;
	SeriesTitleEntry *mySeriesTitleEntry;
	BookNumberEntry *myBookNumberEntry;


	ZLDialogContent *myTagsTab;
	std::vector<BookTagEntry *> myTagEntries;
	bool myTagsDone;

	std::vector<std::string> myNewTags;


	ZLDialogContent *myAuthorsTab;
	std::vector<AuthorDisplayNameEntry *> myAuthorEntries;
	bool myAuthorsDone;


friend class AuthorDisplayNameEntry;
friend class SeriesTitleEntry;
friend class BookNumberEntry;
friend class BookTitleEntry;
friend class BookEncodingEntry;
friend class BookLanguageEntry;
friend class BookTagEntry;
friend class BookInfoApplyAction;
};

inline ZLOptionsDialog &BookInfoDialog::dialog() { return *myDialog; }

#endif /* __BOOKINFODIALOG_H__ */
