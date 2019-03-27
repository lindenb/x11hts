/*
The MIT License (MIT)

Copyright (c) 2019 Pierre Lindenbaum PhD.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/
#ifndef HERSHEY_H
#define HERSHEY_H
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <iostream>
#include <sstream>
#include "config.h"
#include <X11/Xlib.h>

using namespace std;


class Hershey
	{
	private:
		struct Operator
			{
			double x;
			double y;
			int op;
			};
		const int MOVETO;
		const int LINETO;
		std::vector<Operator> array;
	public:
		double scalex;
		double scaley;

		Hershey():MOVETO(0),LINETO(1),scalex(10.0),scaley(10.0)
			{
			}

		virtual ~Hershey() {
			}
	private:
		const char* charToHersheyString(char c)
				{
				switch(c)
					{
					case 'a' : case 'A' : return "  9MWRMNV RRMVV RPSTS"; break;
					case 'b' : case 'B' : return " 16MWOMOV ROMSMUNUPSQ ROQSQURUUSVOV"; break;
					case 'c' : case 'C' : return " 11MXVNTMRMPNOPOSPURVTVVU"; break;
					case 'd' : case 'D' : return " 12MWOMOV ROMRMTNUPUSTURVOV"; break;
					case 'e' : case 'E' : return " 12MWOMOV ROMUM ROQSQ ROVUV"; break;
					case 'f' : case 'F' : return "  9MVOMOV ROMUM ROQSQ"; break;
					case 'g' : case 'G' : return " 15MXVNTMRMPNOPOSPURVTVVUVR RSRVR"; break;
					case 'h' : case 'H' : return "  9MWOMOV RUMUV ROQUQ"; break;
					case 'i' : case 'I' : return "  3PTRMRV"; break;
					case 'j' : case 'J' : return "  7NUSMSTRVPVOTOS"; break;
					case 'k' : case 'K' : return "  9MWOMOV RUMOS RQQUV"; break;
					case 'l' : case 'L' : return "  6MVOMOV ROVUV"; break;
					case 'm' : case 'M' : return " 12LXNMNV RNMRV RVMRV RVMVV"; break;
					case 'n' : case 'N' : return "  9MWOMOV ROMUV RUMUV"; break;
					case 'o' : case 'O' : return " 14MXRMPNOPOSPURVSVUUVSVPUNSMRM"; break;
					case 'p' : case 'P' : return " 10MWOMOV ROMSMUNUQSROR"; break;
					case 'q' : case 'Q' : return " 17MXRMPNOPOSPURVSVUUVSVPUNSMRM RSTVW"; break;
					case 'r' : case 'R' : return " 13MWOMOV ROMSMUNUQSROR RRRUV"; break;
					case 's' : case 'S' : return " 13MWUNSMQMONOOPPTRUSUUSVQVOU"; break;
					case 't' : case 'T' : return "  6MWRMRV RNMVM"; break;
					case 'u' : case 'U' : return "  9MXOMOSPURVSVUUVSVM"; break;
					case 'v' : case 'V' : return "  6MWNMRV RVMRV"; break;
					case 'w' : case 'W' : return " 12LXNMPV RRMPV RRMTV RVMTV"; break;
					case 'x' : case 'X' : return "  6MWOMUV RUMOV"; break;
					case 'y' : case 'Y' : return "  7MWNMRQRV RVMRQ"; break;
					case 'z' : case 'Z' : return "  9MWUMOV ROMUM ROVUV" ; break;

					case '0' : return " 12MWRMPNOPOSPURVTUUSUPTNRM" ; break;
					case '1' : return "  4MWPORMRV" ; break;
					case '2' : return "  9MWONQMSMUNUPTROVUV" ; break;
					case '3' : return " 15MWONQMSMUNUPSQ RRQSQURUUSVQVOU"; break;
					case '4' : return "  7MWSMSV RSMNSVS" ; break;
					case '5' : return " 14MWPMOQQPRPTQUSTURVQVOU RPMTM" ; break;
					case '6' : return " 14MWTMRMPNOPOSPURVTUUSTQRPPQOS" ; break;
					case '7' : return "  6MWUMQV ROMUM" ; break;
					case '8' : return " 19MWQMONOPQQSQUPUNSMQM RQQOROUQVSVUUURSQ" ; break;
					case '9' : return " 14MWUPTRRSPROPPNRMTNUPUSTURVPV" ; break;

			
			
					case '.': return "  6PURURVSVSURU";//210
					case ',': return "  7PUSVRVRUSUSWRY";//211
					case ':': return " 12PURPRQSQSPRP RRURVSVSURU";//212
					case ';': return " 13PURPRQSQSPRP RSVRVRUSUSWRY";//213
					case '!': return " 12PURMRR RSMSR RRURVSVSURU";//214
					case '?': return " 17NWPNRMSMUNUPRQRRSRSQUP RRURVSVSURU";//215
					case '\'':return "  3PTRMRQ";//216
					case '\"':return "  6NVPMPQ RTMTQ";//217
					case '/': return "  3MWVLNW";//220
					case '(': return "  7OVTLRNQPQSRUTW";//221
					case ')': return "  7NUPLRNSPSSRUPW";//222
					case '|': return "  3PTRLRW";//223
					case '#': return " 12MXRLPW RULSW ROPVP ROSVS";//233
					case '*': return "  9JZRLRX RMOWU RWOMU";//728
					case '=': return "  6LXNPVP RNTVT";//226
					case '+': return "  6LXRNRV RNRVR";
					case '-': return "  3KYKRYR";//806
					case '_': return "  3JZJZZZ";//998
					case '[': return " 12MWPHP\\ RQHQ\\ RPHUH RP\\U\\";//1223
					case ']': return " 12MWSHS\\ RTHT\\ ROHTH RO\\T\\";//1224
					case '{': return " 38LWSHQIPJPLRNSP RQIPL RSNRQ RPJQLSNSPRQPRRSSTSVQXPZ RRSSV RPXQ[ RSTRVPXPZQ[S\\"; 
					case '}': return " 38MXQHSITJTLRNQP RSITL RQNRQ RTJSLQNQPRQTRRSQTQVSXTZ RRSQV RTXS[ R QTRVTXTZS[Q\\";
					default: return NULL;
					}
				}
	
	
		void  charToPathOp(char letter)
				{
				size_t i;
				array.clear();
				if(letter==' ') return ;
				const char* s= charToHersheyString(letter);
		
				if(s==NULL) return;
		
				int num_vertices=0;
				for( i=0;i< 3;++i)
					{
					char c=s[i];
					if(c==' ' || c=='\n') continue;
					num_vertices = num_vertices*10+(c-'0');
					}
				num_vertices--;
				i+=2;
				int nop=0;
		
				while(nop<num_vertices)
					{
			
					Operator pathOp;

					pathOp.op=(array.empty()? MOVETO: LINETO);
					char c=s[i++];
					if(c==' ')
						{
						c=s[i++];
						if(c!='R') {fprintf(stderr,"ERROR");break;}//ERROR!
						nop++;
						pathOp.op = MOVETO;
						c=s[i++];
						}
					pathOp.x=c-'R';
					c=s[i++];
					pathOp.y=c-'R';
					nop++;
			
					array.push_back(pathOp);
					}
				}
	
	public:	
	
	
		

		void paint(
			Display* display,
			Window win,
			GC gc,
			const char* s,
			double x, double y,
			double width, double height
			)
			{
			if(s==NULL || width==0 || height==0) return;
			size_t s_length=strlen(s);
			if(s_length==0) return;
			double x1=0;
			double y1=0;
			size_t i=0,n;
			double dx=width/s_length;
			for(i=0;i < s_length;++i)
				{
				charToPathOp(s[i]);
				for(n=0;n< array.size();++n)
					{
					Operator&  p2= array[n];
					double x2= x+ (p2.x/this->scalex)*dx + dx*i +dx/2.0;
					double y2= y+ (p2.y/this->scaley)*height +height/2.0 ;
			
					if(p2.op == LINETO)
						{
						::XDrawLine(display, win, gc, (int)x1, (int)y1,(int)x2, (int)y2);
						x1=x2;
						y1=y2;
						}
					else
						{
						x1=x2;
						y1=y2;
						}
					}
		
				}	
			}
		
	};


#endif

