About this Document
====================

This document aims to specify the inner workings of the lowest level method of communication for the application. Through the CommStream interface, the application should be able to communicate with plaintext messages to the server. The CommStream interface should totally abstract the implementation details from the user, and provide seamless mechanisms for encrypting the communication stream. The CommStream interface should be reentrant.


The Interface
====================

`CommStream()` 
Establishes the memory used by the buffers, and initializes any libraries that need to be initialized.

`~CommStream()` 
Safely terminates any connected clients or servers. Frees any allocated space.

`Error SetEncryptionScheme( enum EncryptionScheme, bool force = false )`
Determines the preferred encryption scheme. If the negotiation for the selected encryption scheme fails, plaintext is used, unless `force` is set to `true`. In such a case, the connection fails with the status `Error::UnsupportedEncryption`.

`Error Connect( std::string address, int port )`
Attempts to connect to the specified address and port. If the connection fails, an appropriate status code is returned. Otherwise, the encryption scheme is negotiated. If the encryption scheme is negotiated successfully, then the connection is successful and this object may be used for communication.

`bool Connected()`
Returns whether or not the stream is connected.

`Error Disconnect( bool force = false )`
Attempts to disconnect the stream. If an important operation is occurring, the connection isn't connected, or the disconnect fails for whatever reason, an error is returned, and only performs the disconnect if `force` is set to `true`.

`Error Listen( [std::string address,] int port )`
Begins listening for connections.

`Error Send( std::string message, [int targetID = -1,] bool important = false )`
Sends a message on the stream. If `important` is set to `true`, then the message shouldn't be interrupted by a safe shutdown. Note, this will not guarantee that the message gets sent in the event of catastrophe, only that the interface will do its best.

`Error Receive( std::string &message, int &id, enum ReceiveMessage &recmsg )`
Gets a message from the queue, along with related metadata, such as control messages, client/server ID, and the message body itself.

Points of Interest
====================

Encryption negotiation:

 - The client will connect to the server over TCP
 - The client then sends a REQ_ENCRYPT message with a magic string that says what the intended encryption is.
 - The server then responds with either an ACK_ENCRYPT message, a CHG_ENCRYPT message, or an INV_ENCRYPT message.
 - In the event of an INV_ENCRYPT, the client should throw an error and abort the connection.
 - In the event of a CHG_ENCRYPT, the client should verify that it can use the chosen encryption, and respond with either an INV_ENCRYPT or ACK_ENCRYPT.
 - In the event of an ACK_ENCRYPT, the client should then respond with an ACK_ENCRYPT. This completes the negotiation.
 - If the server receives an INV_ENCRYPT from the client, the server should choose another acceptable encryption scheme and resubmit a CHG_ENCRYPT message until all options are exhausted.
 - When the server receives an ACK_ENCRYPT, the connection has been established. All future communication over this stream will be encrypted as negotiated.
