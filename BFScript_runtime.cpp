#include<windows.h>
#include<iostream>
#include<fstream>
#include<map>
#include<time.h>
#define defaultsize 1024
using namespace std;
HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
unsigned long long codelen;
unsigned long long ptr = 0;
int memsize = defaultsize;
int* mem = (int*)malloc(memsize * 4 + 4);
int addr = 0;

int fg_color = 0x4, df_color = 0x3, nm_color = 0xb, er_color = 0x40 | nm_color;
typedef struct debug_info
{
	bool step;
	bool debug;
	bool clear_out_str;
} debug_info;
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
};
constexpr int basic_instr_count = 42, nested_instr_count = 6;
string basic_instructions[basic_instr_count] =
{
	"incr;","decr;","zero;","set",
	"getchar;","putchar;","getint;","putint;","newline;","bell;","clearscreen",
	"copy;","move;","swap;","alloc;","free;","clear;","resize;","fill;",
	"exit;","crash;","break;",
	"div;","mod;","add;","sub;","mul;",
	"compare;",
	"sleep;","time;","clock;",
	"next;","prev;","memjump;",
	"jump;",
	"not;","and;","or;","xor;","nand;","nor;","xnor;"
};
string nested_instructions[nested_instr_count] =
{
	"while{","whilezero{","forever{","if{","ifzero{","}"
};

void help(string s, bool param = false)
{
	map<string, string> cell_operations;
	cell_operations["incr"] = "increment current cell";
	cell_operations["decr"] = "decrement current cell";
	cell_operations["zero"] = "set current cell to zero";
	cell_operations["set"] = "set current cell to the followed value";
	map<string, string> input_and_output;
	input_and_output["getchar"] = "ask the user to input a character and store its ASCII value in current cell";
	input_and_output["putchar"] = "take the value of current cell as ASCII value and output the corresponding character";
	input_and_output["getint"] = "ask the user to input a integer and store it in current cell";
	input_and_output["putint"] = "output the value of current cell as an integer";
	input_and_output["newline"] = "start a new line(by printing the newline character)";
	input_and_output["bell"] = "output the BELL(ASCII 7) character, making an alert sound";
	input_and_output["clearscreen"] = "clear all output";
	map<string, string> memory_operations;
	memory_operations["copy"] = "copy current cell, using next cell as offset";
	memory_operations["move"] = "move current cell, using next cell as offset";
	memory_operations["swap"] = "swap current cell, using next cell as offset";
	memory_operations["alloc"] = "allocate memory, current cell as number of cells";
	memory_operations["free"] = "deallocate memory, current cell as number of cells";
	memory_operations["clear"] = "clear all memory(set them to 0)";
	memory_operations["resize"] = "resize memory, current cell as number of cells";
	memory_operations["fill"] = "fill memory with current cell";
	map<string, string> end;
	end["exit"] = "exit with current cell as return value";
	end["crash"] = "call crash";
	end["break"] = "end loop(while or whilezero)";
	map<string, string> arithmetic;
	arithmetic["div"] = "divide current cell by next cell and store the result in the cell after next cell";
	arithmetic["mod"] = "mod current cell by next cell and store the result in the cell after next cell";
	arithmetic["add"] = "add current cell to next cell and store the result in the cell after next cell";
	arithmetic["sub"] = "subtract next cell from current cell and store the result in the cell after next cell";
	arithmetic["mul"] = "multiply current cell by next cell and store the result in the cell after next cell";
	map<string, string> compare;
	compare["compare"] = "compares current cell to next cell, putting result in the cell after next cell(1:>,0:=,-1:<)";
	map<string, string> timing;
	timing["sleep"] = "pauses execution for (value of current cell) milliseconds";
	timing["time"] = "get UNIX time and store it in current cell";
	timing["clock"] = "get program clock (the unit is milliseconds) and store it in current cell";
	map<string, string> pointer_moving;
	pointer_moving["next"] = "move cell pointer to next cell";
	pointer_moving["prev"] = "move cell pointer to previous cell";
	pointer_moving["memjump"] = "move pointer with current cell as offset";
	map<string, string> jump;
	jump["jump"] = "jump program execution with current cell as offset";
	map<string, string> logic;
	logic["not"] = "flips every bit in current cell and stores it in next cell";
	logic["and"] = "AND current cell and next cell, storing the result in the cell after next cell";
	logic["or"] = "OR current cell and next cell, storing the result in the cell after next cell";
	logic["xor"] = "XOR current cell and next cell, storing the result in the cell after next cell";
	logic["nand"] = "NAND current cell and next cell, storing the result in the cell after next cell";
	logic["nor"] = "NOR current cell and next cell, storing the result in the cell after next cell";
	logic["xnor"] = "XNOR current cell and next cell, storing the result in the cell after next cell";
	map<string, string> all;
	all.insert(cell_operations.begin(), cell_operations.end());
	all.insert(input_and_output.begin(), input_and_output.end());
	all.insert(memory_operations.begin(), memory_operations.end());
	all.insert(end.begin(), end.end());
	all.insert(arithmetic.begin(), arithmetic.end());
	all.insert(compare.begin(), compare.end());
	all.insert(timing.begin(), timing.end());
	all.insert(pointer_moving.begin(), pointer_moving.end());
	all.insert(jump.begin(), jump.end());
	all.insert(logic.begin(), logic.end());

	map<string, map<string, string>> help;
	help["cell"] = cell_operations;
	help["io"] = input_and_output;
	help["mem"] = memory_operations;
	help["end"] = end;
	help["arithmetic"] = arithmetic;
	help["compare"] = compare;
	help["timing"] = timing;
	help["pointer"] = pointer_moving;
	help["jump"] = jump;
	help["logic"] = logic;
	help["all"] = all;
	if (param)
	{
		cout << "BFScript runtime.\n";
		s = "all";
	}

	for (pair<string, map<string, string>> p : help)
		if (p.first == s)
		{
			cout << "help on topic \"" << s << "\":\n";
			for (pair<string, string> p2 : p.second)
			{
				if (p2.first == "clearscreen")
				{
					cout << "\t" << p2.first << "\t-" << p2.second << endl;
					continue;
				}
				cout << "\t" << p2.first << "\t\t-" << p2.second << endl;
			}
			return;
		}
	for (pair<string, map<string, string>> p : help)
		for (pair<string, string> p2 : p.second)
			if (p2.first == s)
			{
				cout << "help on item \"" << s << "\":\n";
				cout << "\t" << p2.first << "\t-" << p2.second << endl;
				return;
			}
	cout << "no help found for \"" << s << "\". \n";
	cout << "Try to use --help(-h) on one of the following topics:\n";
	for (pair<string, map<string, string>> p : help)
		cout << "\t" << p.first << endl;
	return;
}

bool match(string s, string subs, unsigned long long start)
{
	unsigned long long len = subs.length();
	if (len > (s.length() - start))
		return false;
	return (subs.compare(s.substr(start, len)) == 0);
}

void color(int c)
{
	SetConsoleTextAttribute(handle, c);
}

void clrscr()
{
	DWORD cCharsWritten;
	DWORD dwConSize;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(handle, &csbi);
	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
	FillConsoleOutputCharacter(handle, (TCHAR)' ', dwConSize, { 0,0 }, &cCharsWritten);
	SetConsoleCursorPosition(handle, { 0, 0 });
}

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

void report_position(string s)
{
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
	} while (current != 0 || code[temp_ptr] != '}');
	while ((code[ptr] == '\t' || code[ptr] == ' ' || code[ptr] == '\n') && ptr < codelen)
		temp_ptr++;
	return temp_ptr;
}

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
	} while (current != 1 || code[temp_ptr] != '{');
	while (code[temp_ptr] != '\t' && code[temp_ptr] != ' ' && code[temp_ptr] != '\n' && code[temp_ptr] != ';' && temp_ptr > 0)
		temp_ptr--;
	if (temp_ptr != 0)
		return temp_ptr + 1;
	else
		return temp_ptr;
}
void execute_basic_instruction(int id, string c, string& out_string, bool output_string = false)
{
	
	
	
	
	
	
	
	
	
	
	int* mptr;
	string s;
	bool minus = false;
	if (id >= 4 && id <= 10)
	{
		fflush(stdin);
		fflush(stdout);
	}
	switch (id)
	{
	case 1:
		mem[addr]++;
		break;
	case 2:
		mem[addr]--;
		break;
	case 3:
		mem[addr] = 0;
		break;
	case 4:
		ptr += 3;
		if (c[ptr] != ' ')
			end(9, c, true);
		ptr++;
		while (c[ptr] == '-')
		{
			minus = (!minus);
			ptr++;
		}
		while (c[ptr] >= '0' && c[ptr] <= '9')
		{
			s += c[ptr];
			ptr++;
		}
		if (c[ptr] != ';')
			end(9, c, true);
		ptr++;
		if (minus)
			mem[addr] = -atoi(s.c_str());
		else
			mem[addr] = atoi(s.c_str());
		break;

	case 5:
		mem[addr] = int(getchar());
		break;
	case 6:
		if (output_string)
			out_string += char(mem[addr]);
		else
			putchar(mem[addr]);
		break;
	case 7:
		cin >> mem[addr];
		break;
	case 8:
		if (output_string)
		{
			char buf[64];
			itoa(mem[addr], buf, 10);
			out_string += buf;
		}
		else
			cout << mem[addr];
		break;
	case 9:
		if (output_string)
			out_string += "\n";
		else
			putchar('\n');
		break;
	case 10:
		putchar(7);
		break;
	case 11:
		if (output_string)
			out_string = "";
		else
			clrscr();
		break;

	case 12:
		if (addr + mem[addr + 1] < memsize)
		{
			if (addr + mem[addr + 1] >= 0)
				mem[addr + mem[addr + 1]] = mem[addr];
			else
				end(1, c, true);
		}
		else
			end(2, c, true);
		break;
	case 13:
		if (addr + mem[addr + 1] < memsize)
		{
			if (addr + mem[addr + 1] >= 0)
			{
				mem[addr] = 0;
				mem[addr + mem[addr + 1]] = mem[addr];
			}
			else
				end(1, c, true);
		}
		else
			end(2, c, true);
		break;
	case 14:
		if (addr + mem[addr + 1] < memsize)
		{
			if (addr + mem[addr + 1] >= 0)
				swap(mem[addr + mem[addr + 1]], mem[addr]);
			else
				end(1, c, true);
		}
		else
			end(2, c, true);
		break;
	case 15:
		memsize += mem[addr];
		mptr = (int*)malloc(memsize);
		if (mptr == nullptr)
			mem[addr + 1] = 0;
		else
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
	case 16:
		if (memsize <= mem[addr])
			end(8, c, true);
		memsize += mem[addr];
		mptr = (int*)malloc(memsize);
		if (mptr == nullptr)
		{
			mem[addr + 1] = 0;
			break;
		}
		else
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
	case 17:
		memset(mem, 0, memsize * 4);
		break;
	case 18:
		memsize = mem[addr];
		mptr = (int*)malloc(memsize);
		if (mptr == nullptr)
			mem[addr + 1] = 0;
		else
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
	case 19:
		fill(mem, mem + memsize * 4, mem[addr]);
		break;

	case 20:
		end(mem[addr], c, false);
		break;
	case 21:
		end(7, c, true);
		break;
	case 22:
		while (c[ptr] != '}')
			ptr++;
		break;
	case 23:
		if (addr > memsize - 3)
			end(2, c, true);
		if (mem[addr + 1] == 0)
			end(5, c, true);
		else
			mem[addr + 2] = mem[addr] / mem[addr + 1];
		break;
	case 24:
		if (addr > memsize - 3)
			end(2, c, true);
		if (mem[addr + 1] == 0)
			end(6, c, true);
		else
			mem[addr + 2] = mem[addr] % mem[addr + 1];
		break;
	case 25:
		if (addr > memsize - 3)
			end(2, c, true);
		else
			mem[addr + 2] = mem[addr] + mem[addr + 1];
		break;
	case 26:
		if (addr > memsize - 3)
			end(2, c, true);
		else
			mem[addr + 2] = mem[addr] - mem[addr + 1];
		break;
	case 27:
		if (addr > memsize - 3)
			end(2, c, true);
		else
			mem[addr + 2] = mem[addr] * mem[addr + 1];
		break;
	case 28:
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

	case 29:
		Sleep(mem[addr]);
		break;
	case 30:
		mem[addr] = time(0);
		break;
	case 31:
		
		mem[addr] = clock();
		break;

	case 32:
		if (addr != memsize - 1)
			addr++;
		else
			end(2, c, true);
		break;
	case 33:
		if (addr != 0)
			addr--;
		else
			end(1, c, true);
		break;
	case 34:
		if (addr + mem[addr + 1] < 0)
			end(1, c, true);
		if (addr + mem[addr + 1] >= memsize)
			end(2, c, true);
		addr += mem[addr + 1];
		break;

	case 35:
		if (ptr + mem[addr] < 0)
			end(10, c, true);
		if (ptr + mem[addr] > codelen - 1)
			end(11, c, true);
		ptr += mem[addr];
		break;

	case 36:
		if (ptr > memsize - 2)
			end(2, c, true);
		mem[addr + 1] = ~mem[addr];
		break;
	case 37:
		if (ptr > memsize - 3)
			end(2, c, true);
		mem[addr + 2] = mem[addr] & mem[addr + 1];
		break;
	case 38:
		if (ptr > memsize - 3)
			end(2, c, true);
		mem[addr + 2] = mem[addr] | mem[addr + 1];
		break;
	case 39:
		if (ptr > memsize - 3)
			end(2, c, true);
		mem[addr + 2] = mem[addr] ^ mem[addr + 1];
		break;
	case 40:
		if (ptr > memsize - 3)
			end(2, c, true);
		mem[addr + 2] = ~(mem[addr] & mem[addr + 1]);
		break;
	case 41:
		if (ptr > memsize - 3)
			end(2, c, true);
		mem[addr + 2] = ~(mem[addr] | mem[addr + 1]);
		break;
	case 42:
		if (ptr > memsize - 3)
			end(2, c, true);
		mem[addr + 2] = ~(mem[addr] ^ mem[addr + 1]);
		break;
	}
}

void execute_nested_instruction(int id, string c)
{
	static bool skip_mark;
	unsigned long long temp_ptr, temp_ptr_2;
	switch (id)
	{
	case 1:
		if (mem[addr] == 0)
		{
			skip_mark = true;
			ptr = next_token_of_matching_closing_bracket(ptr, c);
		}
		else
			ptr += nested_instructions[0].length();
		break;
	case 2:
		if (mem[addr] != 0)
		{
			skip_mark = true;
			ptr = next_token_of_matching_closing_bracket(ptr, c);
		}
		else
			ptr += nested_instructions[1].length();
		break;
	case 3:
		ptr += nested_instructions[2].length();
		break;

	case 4:
		if (mem[addr] == 0)
			ptr = next_token_of_matching_closing_bracket(ptr, c);
		else
			ptr += nested_instructions[3].length();
		break;
	case 5:
		if (mem[addr] != 0)
			ptr = next_token_of_matching_closing_bracket(ptr, c);
		else
			ptr += nested_instructions[4].length();
		break;

	case 6:
		temp_ptr = matching_opening_bracket(ptr, c);
		
		
		if (skip_mark)
		{
			skip_mark = false;
			temp_ptr_2 = ptr;
			while (c[temp_ptr_2] == ' ' || c[temp_ptr_2] == '\t' || c[temp_ptr_2] == '\n')
				temp_ptr++;
			if (temp_ptr_2 == codelen - 1)
				end(0);
			else
				ptr++;
			break;
		}
		if (match(c, "while{", temp_ptr) || match(c, "whilezero{", temp_ptr) || match(c, "forever{", temp_ptr))
			ptr = temp_ptr;
		break;
	}
}

void debug(string code, string out_str)
{
	clrscr();
	cout << mem << endl;
	report_position(code);
	cout << addr << endl;
	for (int i = 0; i < 20; i++)
	{
		if (i == addr)
			color(fg_color);
		else
			color(nm_color);
		cout << mem[i] << "\t";
	}
	cout << endl;
	cout << out_str;
}

void exec(string code, debug_info debug_info = { false,false })
{
	static string out_str = "";
	if (debug_info.clear_out_str)
		out_str = "";
	bool matched = false;
	codelen = code.length();
	ptr = 0;
	while (ptr < codelen - 1)
	{
		if (debug_info.debug)
		{
			debug(code, out_str);
			if (debug_info.step)
				getchar();
			else
				Sleep(500);
		}
		matched = false;
		for (int i = 0; i < basic_instr_count; i++)
			if (match(code, basic_instructions[i], ptr))
			{
				matched = true;
				execute_basic_instruction(i + 1, code, out_str, true);
				if (i != 39)
					ptr += basic_instructions[i].length();
				next_token(code);
				break;
			}
		if (debug_info.debug)
		{
			debug(code, out_str);
			if (debug_info.step)
				getchar();
			else
				Sleep(500);
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
inline void init_mem()
{
	memset(mem, 0, memsize * 4);
}
int main(int argc, char** argv)
{
	debug_info di{ false,false,false };
	system("title BFScript runtime");
	string c = "", concat = "";
	color(df_color);
	if (argc == 1)
	{
		cout << "usage:" << argv[0] << " [parameters]\n";
		cout << "to get help, use \"" << argv[0] << " -h\" or \"" << argv[0] << " --help\"";
		color(nm_color);
		return 0;
	}
	if (argc == 3 && (strcmp(argv[1], "-f") == 0 || strcmp(argv[1], "--file") == 0))
	{
		ifstream fin(argv[2]);
		while (getline(fin, c))
			concat += (c + "\n");
	}
	if (argc == 3 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "--debug") == 0))
	{
		ifstream fin(argv[2]);
		while (getline(fin, c))
			concat += (c + "\n");
		di.debug = true;
	}
	if (argc == 3 && (strcmp(argv[1], "-s") == 0 || strcmp(argv[1], "--step") == 0))
	{
		ifstream fin(argv[2]);
		while (getline(fin, c))
			concat += (c + "\n");
		di.debug = true;
		di.step = true;
	}
	if (argc == 3 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
	{
		help(argv[2]);
		color(nm_color);
		return 0;
	}
	if (argc == 2 && (strcmp(argv[1], "-i") == 0 || strcmp(argv[1], "--interactive") == 0))
	{
		cout << "BFScript interactive shell\nBFScript code:";
		while (getline(cin, c))
			concat += (c + "\n");
	}
	if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
	{
		help("", true);
		color(nm_color);
		return 0;
	}
	init_mem();
	exec(concat, di);
	color(nm_color);
}
