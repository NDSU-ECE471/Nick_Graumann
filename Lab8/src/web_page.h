/*
 * web_page.h
 *
 *  Created on: Apr 7, 2013
 *      Author: Nate
 */

#ifndef WEB_PAGE_H_
#define WEB_PAGE_H_

const char website[]= {""
		"HTTP/1.0 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"	// End of HTTP response
		"<html>\n" // Start of html content
		"<body bgcolor=\"red\">\n"
      "<center>\n"
		"<h1>LED Commander: 471 Edition</h1>\n"
      "<p>By Nick Graumann</p>"
      "<a href=\"led_on\"><input type=\"button\" value=\"Turn on LED\" /></a>\n"
      "<a href=\"led_off\"><input type=\"button\" value=\"Turn off LED\" /></a>\n"
      "</center>\n"
		"</body></html>\n"
};


#endif /* WEB_PAGE_H_ */
