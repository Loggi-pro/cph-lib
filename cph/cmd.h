#pragma once
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


#define _COMMAND_LINE_LEN (1+100)									// for 32 chars

/*
Command token number, define max token it command line, if number of token
typed in command line exceed this value, then prints message about it and
command line not to be parced and 'execute' callback will not calls.
Token is word separate by white space, for example 3 token line:
"IRin> set mode test" */
#define _COMMAND_TOKEN_NMB 8

/*
Define you prompt string here. You can use colors escape code, for highlight you prompt,
for example this prompt will green color (if you terminal supports color)*/
//#define _PROMPT_DEFAULT "\033[32mIRin >\033[0m "	// green color
#define _PROMPT_DEFAULT "\033[32mIRin >\033[0m "	// green color
//#define _PROMPT_DEFAULT "IRin > "

/*
Define prompt text (without ESC sequence, only text) prompt length, it needs because if you use
ESC sequence, it's not possible detect only text length*/
#define _PROMPT_LEN       7

/*Define it, if you wanna use completion functional, also set completion callback in you code,
now if user press TAB calls 'copmlitetion' callback. If you no need it, you can just set
NULL to callback ptr and do not use it, but for memory saving tune,
if you are not going to use it - disable this define.*/
#define _USE_COMPLETE

/*Define it, if you wanna use history. It s work's like bash history, and
set stored value to cmdline, if UP and DOWN key pressed. Using history add
memory consuming, depends from _RING_HISTORY_LEN parametr */
#define _USE_HISTORY

/*
History ring buffer length, define static buffer size.
For saving memory, each entered cmdline store to history in ring buffer,
so we can not say, how many line we can store, it depends from cmdline len,
but memory using more effective. We not prefer dinamic memory allocation for
small and embedded devices. Overhead is 2 char on each saved line*/
#define _RING_HISTORY_LEN 64

/*
Enable Handling terminal ESC sequence. If disabling, then cursor arrow, HOME, END will not work,
use Ctrl+A(B,F,P,N,A,E,H,K,U,C) see README, but decrease code memory.*/
#define _USE_ESC_SEQ

/*
Use snprintf from you standard complier library, but it gives some overhead.
If not defined, use my own u16int_to_str variant, it's save about 800 byte of code size
on AVR (avr-gcc build).
Try to build with and without, and compare total code size for tune library.
*/
//#define _USE_LIBC_STDIO

/*
Enable 'interrupt signal' callback, if user press Ctrl+C */
#define _USE_CTLR_C

/*
Print prompt at 'microrl_init', if enable, prompt will print at startup,
otherwise first prompt will print after first press Enter in terminal
NOTE!: Enable it, if you call 'microrl_init' after your communication subsystem
already initialize and ready to print message */
#undef _ENABLE_INIT_PROMPT

/*
New line symbol */
#define _ENDL_LF

#if defined(_ENDL_CR)
	#define ENDL "\r"
#elif defined(_ENDL_CRLF)
	#define ENDL "\r\n"
#elif defined(_ENDL_LF)
	#define ENDL "\n"
#elif defined(_ENDL_LFCR)
	#define ENDL "\n\r"
#else
	#error "You must define new line symbol."
#endif

/********** END CONFIG SECTION ************/


#if _RING_HISTORY_LEN > 256
	#error "This history implementation (ring buffer with 1 byte iterator) allow 256 byte buffer size maximum"
#endif


/* define the Key codes */
#define KEY_NUL 0 /**< ^@ Null character */
#define KEY_SOH 1 /**< ^A Start of heading, = console interrupt */
#define KEY_STX 2 /**< ^B Start of text, maintenance mode on HP console */
#define KEY_ETX 3 /**< ^C End of text */
#define KEY_EOT 4 /**< ^D End of transmission, not the same as ETB */
#define KEY_ENQ 5 /**< ^E Enquiry, goes with ACK; old HP flow control */
#define KEY_ACK 6 /**< ^F Acknowledge, clears ENQ logon hand */
#define KEY_BEL 7 /**< ^G Bell, rings the bell... */
#define KEY_BS  8 /**< ^H Backspace, works on HP terminals/computers */
#define KEY_HT  9 /**< ^I Horizontal tab, move to next tab stop */
#define KEY_LF  10  /**< ^J Line Feed */
#define KEY_VT  11  /**< ^K Vertical tab */
#define KEY_FF  12  /**< ^L Form Feed, page eject */
#define KEY_CR  13  /**< ^M Carriage Return*/
#define KEY_SO  14  /**< ^N Shift Out, alternate character set */
#define KEY_SI  15  /**< ^O Shift In, resume defaultn character set */
#define KEY_DLE 16  /**< ^P Data link escape */
#define KEY_DC1 17  /**< ^Q XON, with XOFF to pause listings; "okay to send". */
#define KEY_DC2 18  /**< ^R Device control 2, block-mode flow control */
#define KEY_DC3 19  /**< ^S XOFF, with XON is TERM=18 flow control */
#define KEY_DC4 20  /**< ^T Device control 4 */
#define KEY_NAK 21  /**< ^U Negative acknowledge */
#define KEY_SYN 22  /**< ^V Synchronous idle */
#define KEY_ETB 23  /**< ^W End transmission block, not the same as EOT */
#define KEY_CAN 24  /**< ^X Cancel line, MPE echoes !!! */
#define KEY_EM  25  /**< ^Y End of medium, Control-Y interrupt */
#define KEY_SUB 26  /**< ^Z Substitute */
#define KEY_ESC 27  /**< ^[ Escape, next character is not echoed */
#define KEY_FS  28  /**< ^\ File separator */
#define KEY_GS  29  /**< ^] Group separator */
#define KEY_RS  30  /**< ^^ Record separator, block-mode terminator */
#define KEY_US  31  /**< ^_ Unit separator */

#define KEY_DEL 127 /**< Delete (not a real control character...) */

#define IS_CONTROL_CHAR(x) ((x)<=31)

// direction of history navigation
#define _HIST_UP   0
#define _HIST_DOWN 1
// esc seq internal codes
#define _ESC_BRACKET  1
#define _ESC_HOME     2
#define _ESC_END      3


namespace cph {

	#ifdef _USE_HISTORY
// history struct, contain internal variable
// history store in static ring buffer for memory saving
	struct ring_history_t {
		char ring_buf [_RING_HISTORY_LEN];
		int begin;
		int end;
		int cur;
		ring_history_t(): begin(0), end(0), cur(0) {
			memset(this->ring_buf, 0, _RING_HISTORY_LEN);
		}
//*****************************************************************************
// print buffer content on screen
		template <typename STREAM>
		void print_hist (STREAM s) {
			s << "\n";

			for (int i = 0; i < _RING_HISTORY_LEN; i++) {
				if (i == this->begin)
				{ s << "b"; }
				else
				{ s << " "; }
			}

			s << "\n";

			for (int i = 0; i < _RING_HISTORY_LEN; i++) {
				if (isalpha(this->ring_buf[i]))
				{ s << (char)this->ring_buf[i]; }
				else
				{ s << (uint8_t)this->ring_buf[i]; }
			}

			s << "\n";

			for (int i = 0; i < _RING_HISTORY_LEN; i++) {
				if (i == this->end)
				{ s << "e"; }
				else
				{ s << " "; }
			}

			s << "\n";
		}


//*****************************************************************************
// remove older message from ring buffer
		void hist_erase_older () {
			int new_pos = this->begin + this->ring_buf [this->begin] + 1;

			if (new_pos >= _RING_HISTORY_LEN)
			{ new_pos = new_pos - _RING_HISTORY_LEN; }

			this->begin = new_pos;
		}

//*****************************************************************************
// check space for new line, remove older while not space
		bool hist_is_space_for_new (int len) {
			if (this->ring_buf [this->begin] == 0)
			{ return true; }

			if (this->end >= this->begin) {
				if (_RING_HISTORY_LEN - this->end + this->begin - 1 > len)
				{ return true; }
			}	else {
				if (this->begin - this->end - 1 > len)
				{ return true; }
			}

			return false;
		}

//*****************************************************************************
// put line to ring buffer
		void hist_save_line (char* line, int len) {
			if (len > _RING_HISTORY_LEN - 2)
			{ return; }

			while (!hist_is_space_for_new (len)) {
				hist_erase_older ();
			}

			// if it's first line
			if (this->ring_buf [this->begin] == 0)
			{ this->ring_buf [this->begin] = len; }

			// store line
			if (len < _RING_HISTORY_LEN - this->end - 1)
			{ memcpy (this->ring_buf + this->end + 1, line, len); }
			else {
				int part_len = _RING_HISTORY_LEN - this->end - 1;
				memcpy (this->ring_buf + this->end + 1, line, part_len);
				memcpy (this->ring_buf, line + part_len, len - part_len);
			}

			this->ring_buf [this->end] = len;
			this->end = this->end + len + 1;

			if (this->end >= _RING_HISTORY_LEN)
			{ this->end -= _RING_HISTORY_LEN; }

			this->ring_buf [this->end] = 0;
			this->cur = 0;
			#ifdef _HISTORY_DEBUG
			//print_hist ();
			#endif
		}

//*****************************************************************************
// copy saved line to 'line' and return size of line
		int hist_restore_line (char* line, int dir) {
			int cnt = 0;
			// count history record
			int header = this->begin;

			while (this->ring_buf [header] != 0) {
				header += this->ring_buf [header] + 1;

				if (header >= _RING_HISTORY_LEN)
				{ header -= _RING_HISTORY_LEN; }

				cnt++;
			}

			if (dir == _HIST_UP) {
				if (cnt >= this->cur) {
					int header = this->begin;
					int j = 0;

					// found record for 'this->cur' index
					while ((this->ring_buf [header] != 0) && (cnt - j - 1 != this->cur)) {
						header += this->ring_buf [header] + 1;

						if (header >= _RING_HISTORY_LEN)
						{ header -= _RING_HISTORY_LEN; }

						j++;
					}

					if (this->ring_buf[header]) {
						this->cur++;

						// obtain saved line
						if (this->ring_buf [header] + header < _RING_HISTORY_LEN) {
							memset (line, 0, _COMMAND_LINE_LEN);
							memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
						} else {
							int part0 = _RING_HISTORY_LEN - header - 1;
							memset (line, 0, _COMMAND_LINE_LEN);
							memcpy (line, this->ring_buf + header + 1, part0);
							memcpy (line + part0, this->ring_buf, this->ring_buf[header] - part0);
						}

						return this->ring_buf[header];
					}
				}
			} else {
				if (this->cur > 0) {
					this->cur--;
					int header = this->begin;
					int j = 0;

					while ((this->ring_buf [header] != 0) && (cnt - j != this->cur)) {
						header += this->ring_buf [header] + 1;

						if (header >= _RING_HISTORY_LEN)
						{ header -= _RING_HISTORY_LEN; }

						j++;
					}

					if (this->ring_buf [header] + header < _RING_HISTORY_LEN) {
						memcpy (line, this->ring_buf + header + 1, this->ring_buf[header]);
					} else {
						int part0 = _RING_HISTORY_LEN - header - 1;
						memcpy (line, this->ring_buf + header + 1, part0);
						memcpy (line + part0, this->ring_buf, this->ring_buf[header] - part0);
					}

					return this->ring_buf[header];
				} else {
					/* empty line */
					return 0;
				}
			}

			return -1;
		}


	};
	#endif



	template <uint8_t ID>
	class microcmd_t {
		private:
			#ifdef _USE_ESC_SEQ
			char escape_seq;
			bool escape;
			#endif
			#if (defined(_ENDL_CRLF) || defined(_ENDL_LFCR))
			char tmpch;
			#endif
			#ifdef _USE_HISTORY
			ring_history_t ring_hist;          // history object
			#endif
			void _hist_search ( int dir);
			const char* prompt_str;                  // pointer to prompt string
			char cmdline [_COMMAND_LINE_LEN];  // cmdline buffer
			int cmdlen;                        // last position in command line
			int cursor;                        // input cursor
			int (*execute) (int argc, const char* const* argv );              // ptr to 'execute' callback
			char** (*get_completion) (int argc, const char* const* argv );    // ptr to 'completion' callback
			void (*print) (const char*);                                      // ptr to 'print' callback
			#ifdef _USE_CTLR_C
			void (*sigint) (void);
			#endif

			static unsigned int _common_len (char** arr);
			static char* _u16bit_to_str (unsigned int nmb, char* buf);
			//
			int _split (int limit, char const** tkn_arr);

			void _print_prompt();
			inline void _terminal_backspace ();
			inline void _terminal_newline ();
			inline void _terminal_move_cursor (int offset);
			inline void _terminal_reset_cursor ();
			inline void _terminal_print_line (int pos, int cursor);
			void _new_line_handler();
			bool _escape_process ( char ch);
			void _microrl_backspace ();
			void _microrl_insert_char ( int ch);
			bool _microrl_insert_text ( char* text, int len);
			void _microrl_get_complete ();
		public:
			// init internal data, calls once at start up
			void init (void (*print)(const char*));

			// set echo mode (true/false), using for disabling echo for password input
			// echo mode will enabled after user press Enter.
			void set_echo (bool);

			// set pointer to callback complition func, that called when user press 'Tab'
			// callback func description:
			//   param: argc - argument count, argv - pointer array to token string
			//   must return NULL-terminated string, contain complite variant splitted by 'Whitespace'
			//   If complite token found, it's must contain only one token to be complitted
			//   Empty string if complite not found, and multiple string if there are some token
			void set_complete_callback (char** (*get_completion)(int, const char* const*));

			// pointer to callback func, that called when user press 'Enter'
			// execute func param: argc - argument count, argv - pointer array to token string
			void set_execute_callback (int (*execute)(int, const char* const*));

			// set callback for Ctrl+C terminal signal
			#ifdef _USE_CTLR_C
			void set_sigint_callback ( void (*sigintf)(void));
			#endif
			// insert char to cmdline (for example call in usart RX interrupt)
			void insert_char (int ch);
	};












//*****************************************************************************
// split cmdline to tkn array and return nmb of token
	template <uint8_t ID>
	int microcmd_t<ID>::_split (int limit, char const** tkn_arr) {
		int i = 0;
		int ind = 0;

		while (1) {
			// go to the first whitespace (zerro for us)
			while ((this->cmdline [ind] == '\0') && (ind < limit)) {
				ind++;
			}

			if (!(ind < limit)) { return i; }

			tkn_arr[i++] = this->cmdline + ind;

			if (i >= _COMMAND_TOKEN_NMB) {
				return -1;
			}

			// go to the first NOT whitespace (not zerro for us)
			while ((this->cmdline [ind] != '\0') && (ind < limit)) {
				ind++;
			}

			if (!(ind < limit)) { return i; }
		}

		return i;
	}


//*****************************************************************************
	template <uint8_t ID>
	inline void microcmd_t<ID>::_print_prompt() {
		this->print (this->prompt_str);
	}

//*****************************************************************************
	template <uint8_t ID>
	inline void microcmd_t<ID>::_terminal_backspace () {
		this->print ("\033[D \033[D");
	}

//*****************************************************************************
	template <uint8_t ID>
	inline void microcmd_t<ID>::_terminal_newline () {
		this->print (ENDL);
	}

//*****************************************************************************
// convert 16 bit value to string
// 0 value not supported!!! just make empty string
// Returns pointer to a buffer tail
	template <uint8_t ID>
	char* microcmd_t<ID>::_u16bit_to_str (unsigned int nmb, char* buf) {
		char tmp_str [6] = {0,};
		int i = 0, j;

		if (nmb <= 0xFFFF) {
			while (nmb > 0) {
				tmp_str[i++] = (nmb % 10) + '0';
				nmb /= 10;
			}

			for (j = 0; j < i; ++j)
			{ *(buf++) = tmp_str [i - j - 1]; }
		}

		*buf = '\0';
		return buf;
	}


//*****************************************************************************
// set cursor at position from begin cmdline (after prompt) + offset
	template <uint8_t ID>
	inline void microcmd_t<ID>::_terminal_move_cursor (int offset) {
		char str[16] = {0,};
		char* endstr;
		strcpy (str, "\033[");

		if (offset > 0) {
			endstr = _u16bit_to_str (offset, str + 2);
			strcpy (endstr, "C");
		} else if (offset < 0) {
			endstr = _u16bit_to_str (-(offset), str + 2);
			strcpy (endstr, "D");
		} else
		{ return; }

		this->print (str);
	}

//*****************************************************************************
	template <uint8_t ID>
	inline void microcmd_t<ID>::_terminal_reset_cursor () {
		char str[16];
		#ifdef _USE_LIBC_STDIO
		snprintf (str, 16, "\033[%dD\033[%dC", \
		          _COMMAND_LINE_LEN + _PROMPT_LEN + 2, _PROMPT_LEN);
		#else
		char* endstr;
		strcpy (str, "\033[");
		endstr = _u16bit_to_str ( _COMMAND_LINE_LEN + _PROMPT_LEN + 2, str + 2);
		strcpy (endstr, "D\033["); endstr += 3;
		endstr = _u16bit_to_str (_PROMPT_LEN, endstr);
		strcpy (endstr, "C");
		#endif
		this->print (str);
	}

//*****************************************************************************
// print cmdline to screen, replace '\0' to wihitespace
	template <uint8_t ID>
	inline void microcmd_t<ID>::_terminal_print_line (int pos, int cursor) {
		this->print ("\033[K");    // delete all from cursor to end
		char nch [] = {0, 0};
		int i;

		for (i = pos; i < this->cmdlen; i++) {
			nch [0] = this->cmdline [i];

			if (nch[0] == '\0')
			{ nch[0] = ' '; }

			this->print (nch);
		}

		_terminal_reset_cursor ();
		_terminal_move_cursor ( cursor);
	}

//*****************************************************************************
	template <uint8_t ID>
	void microcmd_t<ID>::init (void (*print) (const char*)) {
		memset(this->cmdline, 0, _COMMAND_LINE_LEN);
		this->cmdlen = 0;
		this->cursor = 0;
		this->execute = NULL;
		this->get_completion = NULL;
		#ifdef _USE_CTLR_C
		this->sigint = NULL;
		#endif
		this->prompt_str = _PROMPT_DEFAULT;
		this->print = print;
		#ifdef _ENABLE_INIT_PROMPT
		print_prompt ();
		#endif
	}

//*****************************************************************************
	template <uint8_t ID>
	void microcmd_t<ID>::set_complete_callback (char** (*get_completion)(int,
	        const char* const*)) {
		this->get_completion = get_completion;
	}

//*****************************************************************************
	template <uint8_t ID>
	void microcmd_t<ID>::set_execute_callback (int (*execute)(int, const char* const*)) {
		this->execute = execute;
	}
	#ifdef _USE_CTLR_C
//*****************************************************************************
	template <uint8_t ID>
	void microcmd_t<ID>::set_sigint_callback ( void (*sigintf)(void)) {
		this->sigint = sigintf;
	}
	#endif

	template <uint8_t ID>
	void microcmd_t<ID>::_hist_search ( int dir) {
		#ifdef _USE_HISTORY
		int len = this->ring_hist.hist_restore_line (this->cmdline, dir);

		if (len >= 0) {
			this->cmdline[len] = '\0';
			this->cursor = this->cmdlen = len;
			_terminal_reset_cursor ();
			_terminal_print_line (0, this->cursor);
		}

		#endif
	}

	#ifdef _USE_ESC_SEQ
//*****************************************************************************
// handling escape sequences
	template <uint8_t ID>
	bool microcmd_t<ID>::_escape_process ( char ch) {
		if (ch == '[') {
			this->escape_seq = _ESC_BRACKET;
			return false;
		} else if (this->escape_seq == _ESC_BRACKET) {
			if (ch == 'A') {
				_hist_search (_HIST_UP);
				return true;
			} else if (ch == 'B') {
				_hist_search (_HIST_DOWN);
				return true;
			} else if (ch == 'C') {
				if (this->cursor < this->cmdlen) {
					_terminal_move_cursor (1);
					this->cursor++;
				}

				return true;
			} else if (ch == 'D') {
				if (this->cursor > 0) {
					_terminal_move_cursor (-1);
					this->cursor--;
				}

				return true;
			} else if (ch == '7') {
				this->escape_seq = _ESC_HOME;
				return false;
			} else if (ch == '8') {
				this->escape_seq = _ESC_END;
				return false;
			}
		} else if (ch == '~') {
			if (this->escape_seq == _ESC_HOME) {
				_terminal_reset_cursor ();
				this->cursor = 0;
				return true;
			} else if (this->escape_seq == _ESC_END) {
				_terminal_move_cursor ( this->cmdlen - this->cursor);
				this->cursor = this->cmdlen;
				return true;
			}
		}

		/* unknown escape sequence, stop */
		return true;
	}
	#endif

//*****************************************************************************
// insert len char of text at cursor position
	template <uint8_t ID>
	bool microcmd_t<ID>::_microrl_insert_text ( char* text, int len) {
		int i;

		if (this->cmdlen + len < _COMMAND_LINE_LEN) {
			memmove (this->cmdline + this->cursor + len,
			         this->cmdline + this->cursor,
			         this->cmdlen - this->cursor);

			for (i = 0; i < len; i++) {
				this->cmdline [this->cursor + i] = text [i];

				if (this->cmdline [this->cursor + i] == ' ') {
					this->cmdline [this->cursor + i] = 0;
				}
			}

			this->cursor += len;
			this->cmdlen += len;
			this->cmdline [this->cmdlen] = '\0';
			return true;
		}

		return false;
	}

//*****************************************************************************
// remove one char at cursor
	template <uint8_t ID>
	void microcmd_t<ID>::_microrl_backspace () {
		if (this->cursor > 0) {
			_terminal_backspace ();
			memmove (this->cmdline + this->cursor - 1,
			         this->cmdline + this->cursor,
			         this->cmdlen - this->cursor + 1);
			this->cursor--;
			this->cmdline [this->cmdlen] = '\0';
			this->cmdlen--;
		}
	}




//*****************************************************************************
	template <uint8_t ID>
	unsigned int microcmd_t<ID>::_common_len (char** arr) {
		unsigned int i;
		char* shortest = arr[0];
		unsigned int shortlen = strlen(shortest);

		for (i = 0; arr[i] != NULL; ++i)
			if (strlen(arr[i]) < shortlen) {
				shortest = arr[i];
				shortlen = strlen(shortest);
			}

		for (i = 0; i < shortlen; ++i)
			for (unsigned int j = 0; arr[j] != 0; ++j)
				if (shortest[i] != arr[j][i])
				{ return i; }

		return i;
	}

//*****************************************************************************
	template <uint8_t ID>
	void microcmd_t<ID>::_microrl_get_complete () {
		#ifdef _USE_COMPLETE
		char const* tkn_arr[_COMMAND_TOKEN_NMB];
		char** compl_token;

		if (this->get_completion == NULL) // callback was not set
		{ return; }

		int status = _split (this->cursor, tkn_arr);

		if (this->cmdline[this->cursor - 1] == '\0')
		{ tkn_arr[status++] = ""; }

		compl_token = this->_get_completion (status, tkn_arr);

		if (compl_token[0] != NULL) {
			int i = 0;
			int len;

			if (compl_token[1] == NULL) {
				len = strlen (compl_token[0]);
			} else {
				len = _common_len (compl_token);
				_terminal_newline ();

				while (compl_token [i] != NULL) {
					this->print (compl_token[i]);
					this->print (" ");
					i++;
				}

				_terminal_newline ();
				_print_prompt ();
			}

			if (len) {
				_microrl_insert_text ( compl_token[0] + strlen(tkn_arr[status - 1]),
				                       len - strlen(tkn_arr[status - 1]));

				if (compl_token[1] == NULL)
				{ _microrl_insert_text ( " ", 1); }
			}

			_terminal_reset_cursor ();
			_terminal_print_line ( 0, this->cursor);
		}

		#endif
	}

//*****************************************************************************
	template <uint8_t ID>
	void microcmd_t<ID>::_new_line_handler() {
		char const* tkn_arr [_COMMAND_TOKEN_NMB];
		int status;
		_terminal_newline ();
		#ifdef _USE_HISTORY

		if (this->cmdlen > 0)
		{ this->ring_hist.hist_save_line (this->cmdline, this->cmdlen); }

		#endif
		status = _split (this->cmdlen, tkn_arr);

		if (status == -1) {
			//          this->print ("ERROR: Max token amount exseed\n");
			this->print ("ERROR:too many tokens");
			this->print (ENDL);
		}

		if ((status > 0) && (this->execute != NULL))
		{ this->execute (status, tkn_arr); }

		_print_prompt ();
		this->cmdlen = 0;
		this->cursor = 0;
		memset(this->cmdline, 0, _COMMAND_LINE_LEN);
		#ifdef _USE_HISTORY
		this->ring_hist.cur = 0;
		#endif
	}

//*****************************************************************************

	template <uint8_t ID>
	void microcmd_t<ID>::_microrl_insert_char ( int ch) {
		#ifdef _USE_ESC_SEQ

		if (this->escape) {
			if (_escape_process( ch)) {
				this->escape = false;
			}

			return;
		}

		#endif

		switch (ch) {
			//-----------------------------------------------------
			#ifdef _ENDL_CR
		case KEY_CR:
			_new_line_handler();
			break;

		case KEY_LF:
			break;
			#elif defined(_ENDL_CRLF)

		case KEY_CR:
			this->tmpch = KEY_CR;
			break;

		case KEY_LF:
			if (this->tmpch == KEY_CR)
			{ _new_line_handler(); }

			break;
			#elif defined(_ENDL_LFCR)

		case KEY_LF:
			this->tmpch = KEY_LF;
			break;

		case KEY_CR:
			if (this->tmpch == KEY_LF)
			{ _new_line_handler(); }

			break;
			#else

		case KEY_CR:
			break;

		case KEY_LF:
			_new_line_handler();
			break;
			#endif

		//-----------------------------------------------------
		case KEY_HT:
			_microrl_get_complete ();
			break;

		//-----------------------------------------------------
		case KEY_ESC:
			#ifdef _USE_ESC_SEQ
			this->escape = true;
			#endif
			break;

		//-----------------------------------------------------
		case KEY_NAK: // ^U
			while (this->cursor > 0) {
				_microrl_backspace ();
			}

			_terminal_print_line ( 0, this->cursor);
			break;

		//-----------------------------------------------------
		case KEY_VT:  // ^K
			this->print ("\033[K");
			this->cmdlen = this->cursor;
			break;

		//-----------------------------------------------------
		case KEY_ENQ: // ^E
			_terminal_move_cursor ( this->cmdlen - this->cursor);
			this->cursor = this->cmdlen;
			break;

		//-----------------------------------------------------
		case KEY_SOH: // ^A
			_terminal_reset_cursor ();
			this->cursor = 0;
			break;

		//-----------------------------------------------------
		case KEY_ACK: // ^F
			if (this->cursor < this->cmdlen) {
				_terminal_move_cursor ( 1);
				this->cursor++;
			}

			break;

		//-----------------------------------------------------
		case KEY_STX: // ^B
			if (this->cursor) {
				_terminal_move_cursor ( -1);
				this->cursor--;
			}

			break;

		//-----------------------------------------------------
		case KEY_DLE: //^P
			_hist_search (_HIST_UP);
			break;

		//-----------------------------------------------------
		case KEY_SO: //^N
			_hist_search (_HIST_DOWN);
			break;

		//-----------------------------------------------------
		case KEY_DEL: // Backspace
		case KEY_BS: // ^U
			_microrl_backspace ();
			_terminal_print_line (this->cursor, this->cursor);
			break;

		//-----------------------------------------------------
		case KEY_DC2: // ^R
			_terminal_newline ();
			_print_prompt ();
			_terminal_reset_cursor ();
			_terminal_print_line (0, this->cursor);
			break;
			//-----------------------------------------------------
			#ifdef _USE_CTLR_C

		case KEY_ETX:
			if (this->sigint != NULL)
			{ this->sigint(); }

			break;
			#endif

		//-----------------------------------------------------
		default:
			if (((ch == ' ') && (this->cmdlen == 0)) || IS_CONTROL_CHAR(ch))
			{ break; }

			if (_microrl_insert_text ( (char*)&ch, 1))
			{ _terminal_print_line ( this->cursor - 1, this->cursor); }

			break;
		}
	}


}