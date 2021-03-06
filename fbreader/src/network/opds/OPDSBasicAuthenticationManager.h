/*
 * Copyright (C) 2010 Geometer Plus <contact@geometerplus.com>
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

#ifndef __OPDSBASICAUTHENTICATIONMANAGER_H__
#define __OPDSBASICAUTHENTICATIONMANAGER_H__

#include "../NetworkAuthenticationManager.h"


/*
 * Authentication types:
 *	basic, post
 *
 * `basic` parameters:
 *     - signInURL (signInURL == checkURL)
 *     - signOutURL
 *
 * `post` parameters:
 *     - login string name
 *     - password string name
 *     - signInURL
 *     - checkURL
 *     - signOutURL
 */

/*
 * When signing in and/or trying to access account page there can be following answers:
 *   - HTTP/1.1 200 OK
 *     Content can be used to qualify the result
 *
 *   - HTTP/1.1 302 Found
 *     "Location:" can be used to qualify the result
 *
 *   - HTTP/1.1 4xx
 *     for example: HTTP/1.1 401 Unauthorized
 */

class OPDSBasicAuthenticationManager : public NetworkAuthenticationManager {

public:
	OPDSBasicAuthenticationManager(
		const std::string &siteName,
		const std::string &signInUrl,
		const std::string &signOutUrl
	);

public:
	AuthenticationStatus isAuthorised(bool useNetwork = true);
	std::string authorise(const std::string &pwd); // returns error message
	void logOut();

	std::string networkBookId(const NetworkBookItem &book);
	NetworkItem::URLType downloadLinkType(const NetworkBookItem &book);

public: // Account specific methods (can be called only if authorised!!!)
	const std::string &currentUserName();
	/*bool needsInitialization();
	std::string initialize(); // returns error message
	bool needPurchase(const NetworkBookItem &book); // returns true if link must be purchased before downloading
	std::string purchaseBook(NetworkBookItem &book); // returns error message
	std::string downloadLink(const NetworkBookItem &book); // returns link to download book*/

	//std::string refillAccountLink();
	//std::string currentAccount();

	//const std::string &certificate();

private: // config data
	std::string mySignInUrl;
	std::string mySignOutUrl;

private:
	bool myAccountChecked;

	ZLStringOption myAccountUserNameOption;
};

#endif /* __OPDSBASICAUTHENTICATIONMANAGER_H__ */
