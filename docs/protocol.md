# Monitor Protocol
## Abstract
This markdown file presents a simple and text-based monitor protocol for POP3 servers. The protocol features a line-based structure with specific line endings, uppercase instructions, and limited-length arguments. Authentication is required, and various commands are available for tasks such as retrieving user information, managing connections, and performing user-related actions. The protocol's response format distinguishes between success ("OwO") and error ("UwU") conditions. Overall, this protocol provides an efficient and straightforward approach to monitoring POP3 servers.
## Characteristics
- Text-based protocol
- Line ends with \<CRLF>
- All requests must be only one line
- All words are separated by ONLY one space
- The first word refers to the instruction
- Instructions must be uppercase
- All subsequent strings are considered arguments
- Arguments can be 40 characters long at maximum
- Arguments can be upper and lowercase
- All responses begin with "OwO ", in case of success or "UwU ", in case of an error
- Authentication is required. Who sets up the monitoring server is responsible for setting up at least one user. Without authentication only `LOGIN` and `QUIT` commands are available.
- All responses end with two CRLF octets
- Pipelining is supported
## Commands
- QUIT
	- Closes the connection
- LOGIN \<user> \<password>
	- Needed for authentication. 
- GET_USERS
	- Returns a list of all the users, one per line. Each line obey the following syntax: `<username> <is_online>` where *is_online* can be ONLINE, LOGGING_IN or OFFLINE
	```
	C: GET_USERS
 	S: OwO
	S: user1 OFFLINE
 	S: user2 ONLINE
 	S: user3 ONLINE
 	S: user4 LOGGING_IN
	S:
 	```
- GET_USER \<username>
	- Returns the existence of a user and it's status. For example:
	```
	C: GET_USER user1
	S: UwU User doesn't exists
	S:
	...
	C: GET_USER user2
	S: OwO user2 ONLINE
	S:
	```
- GET_CURR_CONN:
	- Returns an integer indicating how many users are connected to the server (Not including the monitor users).
	Example:
	```
	C: GET_CURR_CONN
	S: OwO 134
	S:
	...
	C: GET_CURR_CONN
	S: UwU Server offline
	S:
	```
- GET_TOTAL_CONN
	- Returns an integer indicating how many connections have been opened. Similar to *GET_CURR_CONN*

- GET_SENT_BYTES
	- Returns an integer representing the number of bytes that have been sent by the server, not counting the one from the monitor protocol.
	Example:
	```
	C: GET_SENT_BYTES
	S: OwO 747772166374
	S:
	```
- ADD_USER \<username> \<password>
	- Creates a new user **for the server**, not for the monitor server. 
	Example:
	```
	C: ADD_USER user_foo 123pass
	S: UwU User already exists
	S:
	...
	C: ADD_USER user_foo 123pass
	S: UwU Maximum number of users reached.
	S:
	...
	C: ADD_USER user_foo 123pass
	S: OwO User created!
	S:
	``` 
 - POPULATE_USER \<username>
	 - Fills the maildir of a user with 10 random mails. Mails have random sizes between 16KB to 64MB.
	 Example:
	```
	C: POPULATE_USER user_foo
	S: UwU User doesn't exist
	S:
	...
	C: POPULATE_USER user1
	S: OwO Maildir populated
	S:
	```
 - DELETE_USER \<username>
	 - Removes the given user. If the user doesn't exist, still returns success.
	 Example:
	```
	C: DELETE_USER user_1
	S: OwO Deleted
	S:
	```
