/*
 * web_page.h
 *
 *  Created on: Apr 7, 2013
 *      Author: Nate
 */

#ifndef WEB_PAGE_H_
#define WEB_PAGE_H_

const uint8_t website[]= {""
		"HTTP/1.0 200 OK\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"	// End of HTTP response
		"<html>\n" // Start of html content
		"<head>\n"
		"<center><h1>Web Server of super uber shiny amazing awesomeness!</h1></center>\n"
		"</body></html>\n"
};


#endif /* WEB_PAGE_H_ */
