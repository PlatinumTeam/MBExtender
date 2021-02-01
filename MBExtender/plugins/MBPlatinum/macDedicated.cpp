//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include <MBExtender/MBExtender.h>

#include <TorqueLib/console/console.h>
#include <TorqueLib/game/trigger.h>
#include <TorqueLib/game/demoGame.h>
#include <TorqueLib/platformMacCarb/macCarbConsole.h>

#include <termios.h>
#include <signal.h>
#include <unistd.h>

MBX_MODULE(MacDedicated);

class StdConsole
{
	bool stdConsoleEnabled;
	bool stdConsoleInputEnabled;
	// true if we're running in the background
	bool inBackground;
	TGE::ConsoleEvent postEvent;

	int stdOut;
	int stdIn;
	int stdErr;
	char inbuf[512];
	S32  inpos;
	bool lineOutput;
	char curTabComplete[512];
	S32  tabCompleteStart;
	char rgCmds[MAX_CMDS][512];
	S32  iCmdIndex;

	// this holds the original terminal state
	// before we messed with it
	struct termios *originalTermState;

	void printf(const char *s, ...);

public:
	StdConsole();
	virtual ~StdConsole();
	void process();
	void enable(bool);
	void enableInput(bool enabled);
	void processConsoleLine(const char *consoleLine);
	static void create();
	static void destroy();
	static bool isEnabled();
	void resetTerminal();
};

StdConsole *stdConsole;

MBX_OVERRIDE_MEMBERFN(void, TGE::MacConsole::enable, (TGE::MacConsole *thisptr, bool enable), originalEnable) {
	if (stdConsole) {
		stdConsole->enable(enable);
		stdConsole->enableInput(enable);
	}
}

MBX_ON_GAME_START(OnStart, ()) {
	StdConsole::create();
}

MBX_ON_GAME_EXIT(OnExit, ()) {
	StdConsole::destroy();
}

MBX_ON_CLIENT_PROCESS(DedUpdate, (uint32_t delta)) {
	stdConsole->process();
}

void StdConsole::create()
{
	if (stdConsole == NULL)
		stdConsole = new StdConsole();
}

void StdConsole::destroy()
{
	if (stdConsole && stdConsole->isEnabled())
		stdConsole->enable(false);

	delete stdConsole;
	stdConsole = NULL;
}

static void signalHandler(int sigtype)
{
	if (sigtype == SIGCONT && stdConsole != NULL)
		stdConsole->resetTerminal();
}

void StdConsole::resetTerminal()
{
	if (stdConsoleEnabled)
	{
		/* setup the proper terminal modes */
		struct termios termModes;
		tcgetattr(stdIn, &termModes);
		termModes.c_lflag &= ~ICANON; // enable non-canonical mode
		termModes.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHOKE);
		termModes.c_cc[VMIN] = 0;
		termModes.c_cc[VTIME] = 5;
		tcsetattr(stdIn, TCSAFLUSH, &termModes);
	}
}

void StdConsole::enable(bool enabled)
{
	if (enabled && !stdConsoleEnabled)
	{
		stdConsoleEnabled = true;

		// install signal handler for sigcont
		signal(SIGCONT, &signalHandler);

		// save the terminal state
		if (originalTermState == NULL)
			originalTermState = new termios;

		tcgetattr(stdIn, originalTermState);

		// put the terminal into our preferred mode
		resetTerminal();

		printf("%s", TGE::Con::getVariable("Con::Prompt"));

	}
	else if (!enabled && stdConsoleEnabled)
	{
		stdConsoleEnabled = false;

		// uninstall signal handler
		signal(SIGCONT, SIG_DFL);

		// reset the original terminal state
		if (originalTermState != NULL)
			tcsetattr(stdIn, TCSANOW, originalTermState);
	}
}

void StdConsole::enableInput( bool enabled )
{
	stdConsoleInputEnabled = enabled;
}

bool StdConsole::isEnabled()
{
	if ( stdConsole )
		return stdConsole->stdConsoleEnabled;

	return false;
}

static void stdConsoleConsumer(TGE::ConsoleLogEntry::Level, const char *line)
{
	stdConsole->processConsoleLine(line);
}

StdConsole::StdConsole()
{
	for (S32 iIndex = 0; iIndex < MAX_CMDS; iIndex ++)
		rgCmds[iIndex][0] = '\0';

	stdOut = dup(1);
	stdIn  = dup(0);
	stdErr = dup(2);

	iCmdIndex = 0;
	stdConsoleEnabled = false;
	stdConsoleInputEnabled = false;
	TGE::Con::addConsumer(stdConsoleConsumer);
	inpos = 0;
	lineOutput = false;
	inBackground = false;
	originalTermState = NULL;
}

StdConsole::~StdConsole()
{
	TGE::Con::removeConsumer(stdConsoleConsumer);

	if (stdConsoleEnabled)
		enable(false);

	if (originalTermState != NULL)
	{
		delete originalTermState;
		originalTermState = NULL;
	}
}

void StdConsole::printf(const char *s, ...)
{
	// Get the line into a buffer.
	static const int BufSize = 4096;
	static char buffer[BufSize];
	va_list args;
	va_start(args, s);
	vsnprintf(buffer, BufSize, s, args);
	va_end(args);
	// Replace tabs with carats, like the "real" console does.
	char *pos = buffer;
	while (*pos) {
		if (*pos == '\t') {
			*pos = '^';
		}
		pos++;
	}
	// Axe the color characters.
	TGE::Con::stripColorChars(buffer);
	// Print it.
	write(stdOut, buffer, strlen(buffer));
	fflush(stdout);
}

void StdConsole::processConsoleLine(const char *consoleLine)
{
	if(stdConsoleEnabled)
	{
		inbuf[inpos] = 0;
		if(lineOutput)
			printf("%s\r\n", consoleLine);
		else
			printf("%c%s\r\n%s%s", '\r', consoleLine, TGE::Con::getVariable("Con::Prompt"), inbuf);
	}
}

void StdConsole::process()
{
	if(stdConsoleEnabled)
	{
		//U16 key;
		char typedData[64];  // damn, if you can type this fast... :-D

		// if we were in the background, reset the terminal
		if (inBackground)
			resetTerminal();
		inBackground = false;

		// see if stdIn has any input waiting
		// mojo for select call
		fd_set rfds;
		struct timeval tv;

		FD_ZERO(&rfds);
		FD_SET(stdIn, &rfds);
		// don't wait at all in select
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		int numEvents = select(stdIn+1, &rfds, NULL, NULL, &tv);
		if (numEvents <= 0)
			// no data available
			return;

		numEvents = read(stdIn, typedData, 64);
		if (numEvents == -1)
			return;

		// TODO LINUX, when debug in qtCreator some times we get false console inputs.
		if( !stdConsoleInputEnabled )
			return;

		typedData[numEvents] = '\0';
		if (numEvents > 0)
		{
			char outbuf[512];
			S32 outpos = 0;

			for (int i = 0; i < numEvents; i++)
			{
				switch(typedData[i])
				{
					case 8:
					case 127:
						/* backspace */
						if (inpos > 0)
						{
							outbuf[outpos++] = '\b';
							outbuf[outpos++] = ' ';
							outbuf[outpos++] = '\b';
							inpos--;
						}
						break;

						// XXX Don't know if we can detect shift-TAB.  So, only handling
						// TAB for now.

					case '\t':
						// In the output buffer, we're going to have to erase the current line (in case
						// we're cycling through various completions) and write out the whole input
						// buffer, so (inpos * 3) + complen <= 512.  Should be OK.  The input buffer is
						// also 512 chars long so that constraint will also be fine for the input buffer.
					{
						// Erase the current line.
						U32 i;
						for (i = 0; i < inpos; i++) {
							outbuf[outpos++] = '\b';
							outbuf[outpos++] = ' ';
							outbuf[outpos++] = '\b';
						}
						// Modify the input buffer with the completion.
						U32 maxlen = 512 - (inpos * 3);
						inpos = TGE::Con::tabComplete(inbuf, inpos, maxlen, true);
						// Copy the input buffer to the output.
						for (i = 0; i < inpos; i++) {
							outbuf[outpos++] = inbuf[i];
						}
					}
						break;

					case '\n':
					case '\r':
						/* new line */
						outbuf[outpos++] = '\r';
						outbuf[outpos++] = '\n';

						inbuf[inpos] = 0;
						outbuf[outpos] = 0;
						printf("%s", outbuf);

						S32 eventSize;
						eventSize = 1;

						strcpy(postEvent.data, inbuf);
						postEvent.size = eventSize + strlen(inbuf) + 1;
						TGE::Game->postEvent(postEvent);

						// If we've gone off the end of our array, wrap
						// back to the beginning
						if (iCmdIndex >= MAX_CMDS)
							iCmdIndex %= MAX_CMDS;

						// Put the new command into the array
						strcpy(rgCmds[iCmdIndex ++], inbuf);

						printf("%s", TGE::Con::getVariable("Con::Prompt"));
						inpos = outpos = 0;
						break;
					case 27:
						// JMQTODO: are these magic numbers keyboard map specific?
						if (typedData[i+1] == 91 || typedData[i+1] == 79)
						{
							i += 2;
							// an arrow key was pressed.
							switch(typedData[i])
							{
								case 'A':
									/* up arrow */
									// Go to the previous command in the cyclic array
									if ((-- iCmdIndex) < 0)
										iCmdIndex = MAX_CMDS - 1;

									// If this command isn't empty ...
									if (rgCmds[iCmdIndex][0] != '\0')
									{
										// Obliterate current displayed text
										for (S32 i = outpos = 0; i < inpos; i ++)
										{
											outbuf[outpos++] = '\b';
											outbuf[outpos++] = ' ';
											outbuf[outpos++] = '\b';
										}

										// Copy command into command and display buffers
										for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos++, outpos++)
										{
											outbuf[outpos] = rgCmds[iCmdIndex][inpos];
											inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
										}
									}
									// If previous is empty, stay on current command
									else if ((++ iCmdIndex) >= MAX_CMDS)
									{
										iCmdIndex = 0;
									}
									break;
								case 'B':
									/* down arrow */
									// Go to the next command in the command array, if
									// it isn't empty
									if (rgCmds[iCmdIndex][0] != '\0' && (++ iCmdIndex) >= MAX_CMDS)
										iCmdIndex = 0;

									// Obliterate current displayed text
									for (S32 i = outpos = 0; i < inpos; i ++)
									{
										outbuf[outpos++] = '\b';
										outbuf[outpos++] = ' ';
										outbuf[outpos++] = '\b';
									}

									// Copy command into command and display buffers
									for (inpos = 0; inpos < (S32)strlen(rgCmds[iCmdIndex]); inpos++, outpos++)
									{
										outbuf[outpos] = rgCmds[iCmdIndex][inpos];
										inbuf [inpos ] = rgCmds[iCmdIndex][inpos];
									}
									break;
								case 'C':
									/* right arrow */
									break;
								case 'D':
									/* left arrow */
									break;
							}
							// read again to get rid of a bad char.
							//read(stdIn, &key, sizeof(char));
							break;
						} else {
							inbuf[inpos++] = typedData[i];
							outbuf[outpos++] = typedData[i];
							break;
						}
						break;
					default:
						inbuf[inpos++] = typedData[i];
						outbuf[outpos++] = typedData[i];
						break;
				}
			}
			if (outpos)
			{
				outbuf[outpos] = 0;
				printf("%s", outbuf);
			}
		}
	}
}
