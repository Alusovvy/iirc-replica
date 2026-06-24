This is my CLI chat project, the idea was to playaround with some networking sockets in C and some data management around memory, file descriptor and so on. 

How to run this?

After running make, and ./server you can use following commands:

	login <username> - will log you in
	/create - allows to create a chatroom
	/join - allows to join chatrooms, will also send all existing messages in chat
	<anything> after /join will be send as a message to chat that was joined
	/join - while in chat, allows to swicht chats.

To-Do for futures if I even get back here at some point:

-Check for memory leaks
-Some error handling, incorrect input handling
-Maybe a client-side to this
-UDP protocol
-IPV4/IPV6 agnostic
