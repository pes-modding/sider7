// A converter between ANSI, Unicode and UTF8

#ifndef _UTF8
#define _UTF8

namespace Utf8 {
    static wchar_t *utf8ToUnicode(const void *src) {
        unsigned int code_page = 65001; //utf-8
        int n, wn;
        wchar_t *ws;
        const char *s = (const char*)src;
        //n = (int)strlen(s);
        wn = MultiByteToWideChar(code_page, 0, s, -1, NULL, 0);
        ws = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (wn+1)*sizeof(wchar_t));
        if (wn > 0) {
            MultiByteToWideChar(code_page, 0, s, -1, ws, wn);
        }
        return ws;
    }

    static char *unicodeToUtf8(const void *src) {
        unsigned int code_page = 65001; //utf-8
        int n, wn;
        char *s;
        const wchar_t *ws = (const wchar_t*)src;
        //wn = (int)wcslen(ws);
        n = WideCharToMultiByte(code_page, 0, ws, -1, NULL, 0, NULL, NULL);
        s = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (n+1)*sizeof(char));
        if (n > 0) {
            WideCharToMultiByte(code_page, 0, ws, -1, s, n, NULL, NULL);
        }
        return s;
    }

    static void free(void *ptr) {
        if (ptr) {
            HeapFree(GetProcessHeap(), 0, ptr);
        }
    }
};

// for ANSI 0x80 - 0x9F
static WORD extendedUnicodes[32] = {
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
	0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
	0x0090, 0x2018, 0x2019, 0x201C, 0x201C, 0x2022, 0x2013, 0x2014,
	0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178
};

class Utf8org {
	public:
		static wchar_t chr_ansiToUnicode(char a) {
			if ((BYTE)a < 0x80 || (BYTE)a > 0x9f)
				return (BYTE)a;
				
			return extendedUnicodes[(BYTE)a - 0x80];
		}
		
		static char chr_unicodeToAnsi(wchar_t a) {
			if ((WORD)a < 0x80 || ((WORD)a > 0x9f && (WORD)a < 0x100))
				return (char)a;
			
			if ((WORD)a < 0x100)
				for (int i = 0; i < 32; i++)
					if (extendedUnicodes[i - 0x80] == (WORD)a)
						return i + 0x80;
						
			return '_';
		}
		
		static BYTE chr_unicodeToUtf8(wchar_t a, BYTE* b) {
			if (a > 0xffff) return 0;
			if (a >= 0x800) {
				b[0] = 0xe0 | ((a >> 12) & 0xf);
				b[1] = 0x80 | ((a >> 6) & 0x3f);
				b[2] = 0x80 | (a & 0x3f);
				return 3;
			}
			if (a >= 0x80) {
				b[0] = 0xc0 | ((a >> 6) & 0x1f);
				b[1] = 0x80 | (a & 0x3f);
				return 2;
			}
				
			b[0] = a & 0x7f;
			return 1;
		}
		
		static wchar_t chr_utf8ToUnicode(BYTE* b) {
			BYTE temp = (b[0] >> 4) & 0xf;
			switch (temp) {
				case 0xf:	// 1111
					return 0; // UCS 4 isn't supported
				
				case 0xe:	// 1110
					//3 bytes character
					return (((b[0] & 0xf) << 12) | ((b[1] & 0x3f) << 6) | (b[2] & 0x3f));
				
				case 0xc: // 1100
				case 0xd: // 1101
					// 2 bytes character
					return (((b[0] & 0x1f) << 6) | (b[1] & 0x3f));
				
				case 8: // 1000
				case 9: // 1001
				case 0xa: // 1010
				case 0xb: // 1011
					// invalid, is the 2nd/3rd/4th byte of an UTF8 character
					return 0;
					
				default:
					// 1 byte character
					return (b[0] & 0x7f);
			}
		}
		
		// Length of an UTF8 string
		static DWORD length(BYTE* utf8) {
			BYTE* p = utf8;
			DWORD counter = 0;
			while (*p != 0) {
				BYTE temp = chr_length(p);
				if (temp == 0) break;
				p += temp;
				counter++;
			}
			return counter;
		}
		
		// Length (in bytes) of an UTF8 string
		static DWORD byteLength(BYTE* utf8) {
			BYTE* p = utf8;
			while (*p != 0) {
				BYTE temp = chr_length(p);
				if (temp == 0) break;
				p += temp;
			}
			return (p - utf8);
		}
		
		// Length of this UTF8 character
		static BYTE chr_length(BYTE* b) {
			BYTE temp = (b[0] >> 4) & 0xf;
			switch (temp) {
				case 0xf:	// 1111
					return 0; // UCS 4 isn't supported
				
				case 0xe:	// 1110
					//3 bytes character
					return 3;
				
				case 0xc: // 1100
				case 0xd: // 1101
					// 2 bytes character
					return 2;
				
				case 8: // 1000
				case 9: // 1001
				case 0xa: // 1010
				case 0xb: // 1011
					// invalid, is the 2nd/3rd/4th byte of an UTF8 character
					return 0;
					
				default:
					// 1 byte character
					return 1;
			}
		}
		
		// ANSI to UTF8
		static BYTE* ansiToUtf8(char* ansi) {
			BYTE* utf8;
			BYTE utf8Bytes[3];
	
			int sLen = strlen(ansi);
		
			BYTE* temp = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 3 * sLen);
			BYTE* p = temp;
		
			for (int i = 0; i < sLen; i++) {
				wchar_t unicode = chr_ansiToUnicode(ansi[i]);
				BYTE utf8Len = chr_unicodeToUtf8(unicode, utf8Bytes);
				memcpy(p, utf8Bytes, utf8Len);
				p += utf8Len;
			}
			
			utf8 = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, p - temp + 1);
			memcpy(utf8, temp, p - temp);
			utf8[p - temp] = 0;
			free(temp);
			
			return utf8;
		}
		
		static BYTE* ansiToUf8(const char* ansi) {
			return ansiToUf8((char*)ansi);
		}
		
		static const BYTE* ansiToUf8C(char* ansi) {
			return (const BYTE*)ansiToUf8(ansi);
		}
		
		static const BYTE* ansiToUf8C(const char* ansi) {
			return (const BYTE*)ansiToUf8((char*)ansi);
		}
		// -----
		
		// UNICODE to UTF8
		
		static BYTE* unicodeToUtf8(wchar_t* unic) {
			BYTE* utf8;
			BYTE utf8Bytes[3];
	
			int sLen = wcslen(unic);
		
			BYTE* temp = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 3 * sLen);
			BYTE* p = temp;
		
			for (int i = 0; i < sLen; i++) {
				BYTE utf8Len = chr_unicodeToUtf8(unic[i], utf8Bytes);
				memcpy(p, utf8Bytes, utf8Len);
				p += utf8Len;
			}
			
			utf8 = (BYTE*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, p - temp + 1);
			memcpy(utf8, temp, p - temp);
			utf8[p - temp] = 0;
			free(temp);
			
			return utf8;
		}
		
		static BYTE* unicodeToUtf8(const wchar_t* unic) {
			return unicodeToUtf8((wchar_t*)unic);
		}
		
		static const BYTE* unicodeToUtf8C(wchar_t* unic) {
			return (const BYTE*)unicodeToUtf8C(unic);
		}
		
		static const BYTE* unicodeToUtf8C(const wchar_t* unic) {
			return (const BYTE*)unicodeToUtf8C((wchar_t*)unic);
		}
		// ----
		
		// ANSI to Unicode
		static wchar_t* ansiToUnicode(char* ansi) {
			int sLen = strlen(ansi);
		
			wchar_t* temp = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(wchar_t) * (sLen + 1));
	
			for (int i = 0; i < sLen; i++) {
				temp[i] = chr_ansiToUnicode(ansi[i]);
			}

			return temp;
		}
		
		static wchar_t* ansiToUnicode(const char* ansi) {
			return ansiToUnicode((char*)ansi);
		}
		
		static const wchar_t* ansiToUnicodeC(char* ansi) {
			return (const wchar_t*)ansiToUnicode(ansi);
		}
		
		static const wchar_t* ansiToUnicodeC(const char* ansi) {
			return (const wchar_t*)ansiToUnicode((char*)ansi);
		}
		
		// ---
		
		// Unicode to ANSI
		static char* unicodeToAnsi(wchar_t* unic) {
			int sLen = wcslen(unic);
		
			char* temp = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * (sLen + 1));
	
			for (int i = 0; i < sLen; i++) {
				temp[i] = chr_unicodeToAnsi(unic[i]);
			}

			return temp;
		}
		
		static char* unicodeToAnsi(const wchar_t* unic) {
			return unicodeToAnsi((wchar_t*)unic);
		}
		
		static const char* unicodeToAnsiC(wchar_t* unic) {
			return (const char*)unicodeToAnsi(unic);
		}
		
		static const char* unicodeToAnsiC(const wchar_t* unic) {
			return (const char*)unicodeToAnsi((wchar_t*)unic);
		}
		// ---
		
		// UTF8 to Unicode
		static wchar_t* utf8ToUnicode(BYTE* utf8) {
			int sLen = length(utf8);
			
			wchar_t* temp = (wchar_t*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(wchar_t) * (sLen + 1));
			BYTE* p = utf8;
			int i = 0;
		
			while (*p != 0) {
				temp[i] = chr_utf8ToUnicode(p);
				p += chr_length(p);
				i++;
			}
			
			return temp;
		}
		
		static wchar_t* utf8ToUnicode(const BYTE* utf8) {
			return utf8ToUnicode((BYTE*)utf8);
		}
		
		static const wchar_t* utf8ToUnicodeC(BYTE* utf8) {
			return (const wchar_t*)utf8ToUnicode(utf8);
		}
		
		static const wchar_t* utf8ToUnicodeC(const BYTE* utf8) {
			return (const wchar_t*)utf8ToUnicode((BYTE*)utf8);
		}
		// ---
		
		// UTF8 to ANSI
		static char* utf8ToAnsi(BYTE* utf8) {
			int sLen = length(utf8);
			
			char* temp = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(char) * (sLen + 1));
			BYTE* p = utf8;
			int i = 0;
		
			while (*p != 0) {
				temp[i] = chr_unicodeToAnsi(chr_utf8ToUnicode(p));
				p += chr_length(p);
				i++;
			}
			
			return temp;
		}
		
		static char* utf8ToAnsi(const BYTE* utf8) {
			return utf8ToAnsi((BYTE*)utf8);
		}
		
		static const char* utf8ToAnsiC(BYTE* utf8) {
			return (const char*)utf8ToAnsi(utf8);
		}
		
		static const char* utf8ToAnsiC(const BYTE* utf8) {
			return (const char*)utf8ToAnsi((BYTE*)utf8);
		}
		// ---
		
		
		static void fAnsiToUnicode(wchar_t* existingArray, char* ansi) {
			wchar_t* temp = ansiToUnicode(ansi);
			wcscpy(existingArray, temp);
			free(temp);
			return;
		}
		
		static void fAnsiToUtf8(BYTE* existingArray, char* ansi) {
			BYTE* temp = ansiToUtf8(ansi);
			memcpy(existingArray, temp, byteLength(temp));
			free(temp);
			return;
		}
		
		static void fUtf8ToUnicode(wchar_t* existingArray, BYTE* utf8) {
			wchar_t* temp = utf8ToUnicode(utf8);
			wcscpy(existingArray, temp);
			free(temp);
			return;
		}
		
		static void fUtf8ToUnicode(wchar_t* existingArray, char* ansi) {
			wchar_t* temp = utf8ToUnicode((BYTE*)ansi);
			wcscpy(existingArray, temp);
			free(temp);
			return;
		}
		
		static void fUtf8ToAnsi(char* existingArray, BYTE* utf8) {
			char* temp = utf8ToAnsi(utf8);
			strcpy(existingArray, temp);
			free(temp);
			return;
		}
		
		static void fUnicodeToUtf8(BYTE* existingArray, wchar_t* unic) {
			BYTE* temp = unicodeToUtf8(unic);
			memcpy(existingArray, temp, byteLength(temp));
			free(temp);
			return;
		}
		
		static void fUnicodeToAnsi(char* existingArray, const wchar_t* unic) {
			char* temp = unicodeToAnsi(unic);
			strcpy(existingArray, temp);
			free(temp);
			return;
		}
		// ---
		
		static void free(void* a) {
			HeapFree(GetProcessHeap(), 0, a);
		}
		
        /*
		static void free(char* a) {
			HeapFree(GetProcessHeap(), 0, (BYTE*)a);
		}
		
		static void free2(wchar_t* a) {
			HeapFree(GetProcessHeap(), 0, (BYTE*)a);
		}
        */
};
	

#endif

