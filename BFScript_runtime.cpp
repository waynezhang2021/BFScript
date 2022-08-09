#include<windows.h>
#include<iostream>
#include<fstream>
#include<conio.h>
#include<csignal>
#include<time.h>
#define defaultsize 1024
using namespace std;
HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);//output handle
unsigned long long codelen;
unsigned long long ptr = 0;//code parsing pointer
int memsize = defaultsize;//memsize indicator
int* mem = (int*)malloc(memsize * 4 + 4);//default memory pointer
int addr = 0;//address pointer
int fg_color = 0x4, df_color = 0x3, nm_color = 0xb, er_color = 0x40 | nm_color;
string error_reason[12] =
{
	"no error",
	"memory pointer out of left bound",
	"memory pointer out of right bound",
	"mismatched \"{}\"",
	"unexpected EOF while running",
	"division by 0",
	"modulo by 0",
	"crash called",
	"negative or zero memory memsize",
	"invalid instruction",
	"out of program left bound while jumping",
	"out of program right bound while jumping"
};//error reason strings
constexpr int basic_instr_count = 40, nested_instr_count = 6;
string basic_instructions[basic_instr_count] =
{
	"incr;","decr;","zero;",
	"getchar;","putchar;","getint;","putint;","newline;","bell;","clearscreen",
	"copy;","move;","swap;","alloc;","free;","clear;","resize;","fill;",
	"exit;","crash;",
	"div;","mod;","add;","sub;",
	"compare;",
	"sleep;","time;","clock;",
	"next;","prev;","memjump;",
	"jump;",
	"not;","and;","or;","xor;","nand;","nor;","xnor;",
	"set"
};//basic instructions:sequencial execution
string nested_instructions[nested_instr_count] =
{
	"while{","whilezero{","forever{","if{","ifzero{","}"
};//nestable instructions:loop and conditional
//colors:foreground-for stress,default-runtime color,normal-console color,error-when the code crashes,also used for debug
//match substring
bool match(string s, string subs, unsigned long long start)
{
	unsigned long long len = subs.length();
	if (len > (s.length() - start))
		return false;
	return (subs.compare(s.substr(start, len)) == 0);
}
//set console color
void color(int c)
{
	SetConsoleTextAttribute(handle, c);
}
//clear screen
void clrscr()
{
	HANDLE hConsole;
	COORD coordScreen = { 0, 0 };
	DWORD cCharsWritten;
	DWORD dwConSize;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hConsole, &csbi);
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(hConsole, (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);
}
//error report function
void error_at(string s)
{
	cout << "\nat:";
	for (int n = 0; n<int(s.length()); n++)
	{
		if (n != ptr)
			color(df_color);
		else
			color(fg_color);
		cout << s[n];
	}
	color(nm_color);
}
//exit function
void end(int val, string code = "", bool iserror = false)
{
	if (val == 0)
	{
		color(df_color);
		cout << "\nBFScript:exit 0(normal return)";
		color(nm_color);
		exit(0);
	}
	if (iserror && val != 0)
	{
		color(er_color);
		cout << "\nBFScript:exit " << val;
		cout << "(runtime error,reason=\"";
		cout << error_reason[val] << "\")";
		color(nm_color);
		error_at(code);
		exit(val);
	}
	if (!iserror)
	{
		color(df_color);
		cout << "\nBFScript:exit " << val << "(user defined return)";
		color(nm_color);
		exit(val);
	}
}
void next_token(string code, bool allow_semicolon = true)
{
	if (allow_semicolon)
		while ((code[ptr] == ';' || code[ptr] == '\t' || code[ptr] == ' ' || code[ptr] == '\n') && ptr < codelen)
			ptr++;
	else
		while ((code[ptr] == '\t' || code[ptr] == ' ' || code[ptr] == '\n') && ptr < codelen)
			ptr++;
}
//as the function name
unsigned long long next_token_of_matching_closing_bracket(unsigned long long pos, string code)
{
	unsigned long long temp_ptr = pos;
	int current = 0;
	do
	{
		temp_ptr++;
		if (code[temp_ptr] == '}')
			current--;
		if (code[temp_ptr] == '{')
			current++;
	}
	while (current != 0 || code[temp_ptr] != '}');
	while ((code[ptr] == '\t' || code[ptr] == ' ' || code[ptr] == '\n') && ptr < codelen)
		temp_ptr++;
	return temp_ptr;
}
//as the function name
unsigned long long matching_opening_bracket(unsigned long long pos, string code)
{
	unsigned long long temp_ptr = pos;
	int current = 0;
	do
	{
		temp_ptr--;
		if (code[temp_ptr] == '}')
			current--;
		if (code[temp_ptr] == '{')
			current++;
	}
	while (current != 1 || code[temp_ptr] != '{');
	while (code[temp_ptr] != '\t' && code[temp_ptr] != ' ' && code[temp_ptr] != '\n' && code[temp_ptr] != ';' && temp_ptr > 0)
		temp_ptr--;
	if (temp_ptr != 0)
		return temp_ptr + 1;
	else
		return temp_ptr;
}
void execute_basic_instruction(int id, string c)
{
	//	"incr;","decr;","zero;",
	//	"getchar;","putchar;","getint;","putint;","newline;","bell;",
	//	"copy;","move;","swap;","alloc;","free;","clear;","resize;","fill;",
	//	"exit;","crash;",
	//	"div;","mod;","add;","sub;",
	//	"compare;",
	//	"sleep;","time;","clock;",
	//	"next;","prev;","memjump",
	//	"jump;",
	//	"not;","and;","or;","xor;","nand;","nor;","xnor;"
	int* mptr;
	string s;
	if (id >= 4 && id <= 9)
	{
		fflush(stdin);
		fflush(stdout);
	}
	switch (id)
	{
		case 1://incr
			mem[addr]++;
			break;
		case 2://decr
			mem[addr]--;
			break;
		case 3://zero
			mem[addr] = 0;
			break;

		case 4://getchar
			mem[addr] = int(getchar());
			break;
		case 5://putchar
			putchar(mem[addr]);
			break;
		case 6://getint
			cin >> mem[addr];
			break;
		case 7://putint
			cout << mem[addr];
			break;
		case 8://newline
			putchar('\n');
			break;
		case 9://bell
			putchar(7);
			break;
		case 10://clearscreen
			clrscr();
			break;

		case 11://copy
			if (addr == memsize - 1)
				end(2, c, true);//out of right bound
			if (addr + mem[addr + 1] < memsize)
			{
				if (addr + mem[addr + 1] >= 0)
					mem[addr + mem[addr + 1]] = mem[addr];
				else
					end(1, c, true);//out of left bound
			}
			else
				end(2, c, true);//out of right bound
			break;
		case 12://move
			if (addr == memsize - 1)
				end(2, c, true);//out of right bound
			if (addr + mem[addr + 1] < memsize)
			{
				if (addr + mem[addr + 1] >= 0)
				{
					mem[addr] = 0;
					mem[addr + mem[addr + 1]] = mem[addr];
				}
				else
					end(1, c, true);//out of left bound
			}
			else
				end(2, c, true);//out of right bound
			break;
		case 13://swap
			if (addr == memsize - 1)
				end(2, c, true);//out of right bound
			if (addr + mem[addr + 1] < memsize)
			{
				if (addr + mem[addr + 1] >= 0)
					swap(mem[addr + mem[addr + 1]], mem[addr]);
				else
					end(1, c, true);//out of left bound
			}
			else
				end(2, c, true);//out of right bound
			break;
		case 14://alloc
			memsize += mem[addr];
			mptr = (int*)malloc(memsize);
			if (mptr == nullptr)//allocation failure
				mem[addr + 1] = 0;
			else//success
			{
				memset(mptr, 0, memsize * 4);
				if (addr >= memsize)
					end(2, c, true);
				mem[addr + 1] = 1;
				memcpy(mptr, mem, 4 * (memsize - mem[addr]));
				free(mem);
				mem = mptr;
			}
			break;
		case 15://free
			if (memsize <= mem[addr])
				end(8, c, true);
			memsize += mem[addr];
			mptr = (int*)malloc(memsize);
			if (mptr == nullptr)//allocation failure
			{
				mem[addr + 1] = 0;
				break;
			}
			else//success
			{
				memset(mptr, 0, memsize * 4);
				if (addr >= memsize)
					end(2, c, true);
				mem[addr + 1] = 1;
				memcpy(mptr, mem, 4 * memsize);
				free(mem);
				mem = mptr;
				break;
			}
		case 16://clear
			memset(mem, 0, memsize * 4);
			break;
		case 17://resize
			memsize = mem[addr];
			mptr = (int*)malloc(memsize);
			if (mptr == nullptr)//allocation failure
				mem[addr + 1] = 0;
			else//success
			{
				memset(mptr, 0, memsize * 4);
				if (addr >= memsize)
					end(2, c, true);
				mem[addr + 1] = 1;
				memcpy(mptr, mem, 4 * (memsize - mem[addr]));
				free(mem);
				mem = mptr;
			}
			break;
		case 18://fill
			fill(mem, mem + memsize * 4, mem[addr]);
			break;

		case 19://exit
			end(mem[addr], c, false);
			break;
		case 20://crash
			end(7, c, true);
			break;

		case 21://div
			if (addr > memsize - 3)
				end(2, c, true);
			if (mem[addr + 1] == 0)
				end(5, c, true);
			else
				mem[addr + 2] = mem[addr] / mem[addr + 1];
			break;
		case 22://mod
			if (addr > memsize - 3)
				end(2, c, true);
			if (mem[addr + 1] == 0)
				end(6, c, true);
			else
				mem[addr + 2] = mem[addr] % mem[addr + 1];
			break;
		case 23://add
			if (addr > memsize - 3)
				end(2, c, true);
			else
				mem[addr + 2] = mem[addr] + mem[addr + 1];
			break;
		case 24://sub
			if (addr > memsize - 3)
				end(2, c, true);
			else
				mem[addr + 2] = mem[addr] - mem[addr + 1];
			break;

		case 25://compare
			if (addr > memsize - 3)
				end(2, c, true);
			else
			{
				if (mem[addr] > mem[addr + 1])
					mem[addr + 2] = 1;
				else
				{
					if (mem[addr] == mem[addr + 1])
						mem[addr + 2] = 0;
					if (mem[addr] < mem[addr + 1])
						mem[addr + 2] = -1;
				}
			}
			break;

		case 26://sleep
			Sleep(mem[addr]);
			break;
		case 27://time
			mem[addr] = time(0);
			break;
		case 28://clock
			//note:For Windows,CLOCKS_PER_SEC is usually 1000;For *NIX users,divide clock() by a number to adjust it.
			mem[addr] = clock();
			break;

		case 29://next
			if (addr != memsize - 1)
				addr++;
			else
				end(2, c, true);
			break;
		case 30://prev
			if (addr != 0)
				addr--;
			else
				end(1, c, true);
			break;
		case 31://memjump
			if (addr + mem[addr + 1] < 0)
				end(1, c, true);
			if (addr + mem[addr + 1] >= memsize)
				end(2, c, true);
			addr += mem[addr + 1];
			break;

		case 32://jump
			if (ptr + mem[addr] < 0)
				end(10, c, true);
			if (ptr + mem[addr] > codelen - 1)
				end(11, c, true);
			ptr += mem[addr];
			break;

		case 33://not
			if (ptr > memsize - 2)
				end(2, c, true);
			mem[addr + 1] = ~mem[addr];
			break;
		case 34://and
			if (ptr > memsize - 3)
				end(2, c, true);
			mem[addr + 2] = mem[addr] & mem[addr + 1];
			break;
		case 35://or
			if (ptr > memsize - 3)
				end(2, c, true);
			mem[addr + 2] = mem[addr] | mem[addr + 1];
			break;
		case 36://xor
			if (ptr > memsize - 3)
				end(2, c, true);
			mem[addr + 2] = mem[addr] ^ mem[addr + 1];
			break;
		case 37://nand
			if (ptr > memsize - 3)
				end(2, c, true);
			mem[addr + 2] = ~(mem[addr] & mem[addr + 1]);
			break;
		case 38://nor
			if (ptr > memsize - 3)
				end(2, c, true);
			mem[addr + 2] = ~(mem[addr] | mem[addr + 1]);
			break;
		case 39://xnor
			if (ptr > memsize - 3)
				end(2, c, true);
			mem[addr + 2] = ~(mem[addr] ^ mem[addr + 1]);
			break;

		case 40://set
			ptr += 3;
			if (c[ptr] != ' ')
				end(9, c, true);//error 9:invalid instruction
			ptr++;
			while (c[ptr] >= '0' && c[ptr] <= '9')
			{
				s += c[ptr];
				ptr++;
			}
			if (c[ptr] != ';')
				end(9, c, true);//invalid instruction
			ptr++;
			mem[addr] = atoi(s.c_str());
			break;
	}
}
//execute nested instruction:loop,conditional,closing "}"
void execute_nested_instruction(int id, string c)
{
	unsigned long long temp_ptr;
	switch (id)
	{
		case 1://while
			if (mem[addr] == 0)
				ptr = next_token_of_matching_closing_bracket(ptr, c);
			else
				ptr += nested_instructions[0].length();
			break;
		case 2://whilezero
			if (mem[addr] != 0)
				ptr = next_token_of_matching_closing_bracket(ptr, c);
			else
				ptr += nested_instructions[1].length();
			break;
		case 3://forever
			ptr += nested_instructions[2].length();
			break;

		case 4://if
			if (mem[addr] == 0)
				ptr = next_token_of_matching_closing_bracket(ptr, c);
			else
				ptr += nested_instructions[3].length();
			break;
		case 5://ifzero
			if (mem[addr] != 0)
				ptr = next_token_of_matching_closing_bracket(ptr, c);
			else
				ptr += nested_instructions[4].length();
			break;

		case 6://closing bracket
			temp_ptr = matching_opening_bracket(ptr, c);
			if (match(c, "while{", temp_ptr) || match(c, "whilezero{", temp_ptr) || match(c, "forever{", temp_ptr))
				ptr = temp_ptr;
			break;
	}
}
//execute code
void exec(string code)
{
	bool matched = false;
	codelen = code.length();
	ptr = 0;
	while (ptr < codelen - 1)
	{
		matched = false;
		for (int i = 0; i < basic_instr_count; i++)
			if (match(code, basic_instructions[i], ptr))
			{
				matched = true;
				execute_basic_instruction(i + 1, code);
				if (i != 39)
					ptr += basic_instructions[i].length();
				next_token(code);
				break;
			}
		for (int i = 0; i < nested_instr_count; i++)
			if (match(code, nested_instructions[i], ptr))
			{
				matched = true;
				execute_nested_instruction(i + 1, code);
				next_token(code, false);
				break;
			}
		if (matched == false)
			end(9, code, true);
	}
}
void init_mem()
{
	memset(mem, 0, memsize * 4);
}
int main()
{
	system("title BFScript runtime");
	string c = "", concat = "";
	color(df_color);
	cout << "BFScript interactive shell\nBFScript code:";
	while (getline(cin, c))
		concat += (c + "\n");
	init_mem();
	exec(concat);
	color(nm_color);
}
