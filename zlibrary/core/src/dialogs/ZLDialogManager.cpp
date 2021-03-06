/*
 * Copyright (C) 2004-2010 Geometer Plus <contact@geometerplus.com>
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

#include "ZLDialogManager.h"
#include "ZLProgressDialog.h"

const ZLResourceKey ZLDialogManager::OK_BUTTON("ok");
const ZLResourceKey ZLDialogManager::CANCEL_BUTTON("cancel");
const ZLResourceKey ZLDialogManager::YES_BUTTON("yes");
const ZLResourceKey ZLDialogManager::NO_BUTTON("no");
const ZLResourceKey ZLDialogManager::APPLY_BUTTON("apply");

const ZLResourceKey ZLDialogManager::COLOR_KEY("color");
const ZLResourceKey ZLDialogManager::DIALOG_TITLE("title");

ZLDialogManager *ZLDialogManager::ourInstance = 0;

ZLDialogManager &ZLDialogManager::Instance() {
	return *ourInstance;
} 

bool ZLDialogManager::isInitialized() {
	return ourInstance != 0;
}

void ZLDialogManager::deleteInstance() {
	if (ourInstance != 0) {
		delete ourInstance;
		ourInstance = 0;
	}
}

ZLDialogManager::ZLDialogManager() {
}

ZLDialogManager::~ZLDialogManager() {
}

const ZLResource &ZLDialogManager::resource() {
	return ZLResource::resource("dialog");
}

const std::string &ZLDialogManager::buttonName(const ZLResourceKey &key) {
	return resource()["button"][key].value();
}

const std::string &ZLDialogManager::dialogTitle(const ZLResourceKey &key) {
	return resource()[key][DIALOG_TITLE].value();
}

const std::string &ZLDialogManager::dialogMessage(const ZLResourceKey &key) {
	return resource()[key]["message"].value();
}

const std::string &ZLDialogManager::waitMessageText(const ZLResourceKey &key) {
	return resource()["waitMessage"][key].value();
}

void ZLDialogManager::informationBox(const ZLResourceKey &key) const {
	informationBox(key, dialogMessage(key));
}

void ZLDialogManager::informationBox(const ZLResourceKey &key, const std::string &message) const {
	informationBox(dialogTitle(key), message);
}

void ZLDialogManager::errorBox(const ZLResourceKey &key) const {
	errorBox(key, dialogMessage(key));
}

int ZLDialogManager::questionBox(const ZLResourceKey &key, const ZLResourceKey &button0, const ZLResourceKey &button1, const ZLResourceKey &button2) const {
	return questionBox(key, dialogMessage(key), button0, button1, button2);
}

shared_ptr<ZLProgressDialog> ZLDialogManager::createProgressDialog(const ZLResourceKey &key __attribute__ ((__unused__))) const {
	return 0;
}

void ZLDialogManager::wait(const ZLResourceKey &key __attribute__ ((__unused__)), ZLRunnable &runnable __attribute__ ((__unused__))) const {
//FIXME later
	runnable.run();
    /*
	shared_ptr<ZLProgressDialog> dialog = createProgressDialog(key);
	if (!dialog.isNull()) {
		dialog->run(runnable);
	} else {
		runnable.run();
	}
    */
}

shared_ptr<ZLOpenFileDialog> ZLDialogManager::createOpenFileDialog(const std::string &directoryPath __attribute__ ((__unused__)), const ZLOpenFileDialog::Filter &filter __attribute__ ((__unused__))) const {
	return 0;
}
