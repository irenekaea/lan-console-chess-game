# LAN Console Chess Game

This project contains the implementations to create a local area network chess game. It includes the source code to create the game server host and client executable. The game is able to support multiple players and spectators (tested up to 20 users online). This is an assignment I did when I am studying in DigiPen Institude of Technology Singapore. 

### Compliation
- Please compile in Visual Studio 2015
- To create the server executable, set the compile mode to Release
- To create the client executable, set the compile mode to Client Release

### Running The Server
- Server will prompt the user for the port to connect to and the chess server will start

### Running The Client
- Program will prompt user for the port and ip address to connect to.
- After connection, program will prompt the user for messages to 
send to the server.
- Type 'help' to display the list of commands
- To terminate the program, type "quit".

In the event of server disconnects, client will be notified, press enter to exit the program.