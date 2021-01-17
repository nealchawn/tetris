/**
 defines _
 GLIBCXX_USE_NANOSLEEP
 UNICODE

 g++.exe -Wall -std=c++11 -pthread -D_GLIBCXX_USE_NANOSLEEP -DUNICODE -g  -c C:\Users\zzz\code\c++\Tetris\main.cpp -o C:\Users\zzz\code\c++\Tetris\obj\Debug\main.o
 g++.exe  -o C:\Users\zzz\code\c++\Tetris\bin\Debug\Tetris.exe C:\Users\zzz\code\c++\Tetris\obj\Debug\main.o  -pthread



**/
#include <thread>
#include <iostream>
#include <chrono>
#include <vector>


#include <Windows.h>

using namespace std;

wstring tetromino[7];


int n_field_width = 12;
int n_field_height = 18;
unsigned char *p_field = nullptr;

// for terminal screen
wchar_t *screen = nullptr;
HANDLE h_console;
DWORD dw_bytes_written = 0;


// Terminal / Console screen size
int n_screen_width = 80;
int n_screen_height = 30;

// Function Prototypes
void create_assets();
int Rotate(int px, int py, int r);
void create_playing_field();
void init_command_line_screen();
bool does_piece_fit(int n_tetromino, int n_rotation, int n_pos_x, int n_pos_y);
void resize_screen();

int main(){

  create_assets();
  create_playing_field();
  init_command_line_screen();
  resize_screen();


  // Game Logic
  bool b_game_over = false;

  int n_current_piece = 0;
  int n_current_rotation = 0;
  int n_current_x = n_field_width / 2;
  int n_current_y = 0;

  bool b_key[4];
  bool b_rotate_hold = false;

  int n_speed = 20;
  int n_speed_counter = 0;
  bool b_force_down = false;
  int n_piece_count = 0;
  int n_score = 0;


  vector<int> v_lines;

  // Main game loop
  while (!b_game_over){

    // Game Timing
    //std::this_thread::sleep_for(50ms);
    this_thread::sleep_for(std::chrono::milliseconds(50));
    n_speed_counter++;
    b_force_down = (n_speed_counter == n_speed);

    // Input


    for(int k = 0; k < 4; k++){                                // R  L  D Z
        b_key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;
    }

    // Game Logic
    n_current_x += ( b_key[0] && does_piece_fit(n_current_piece, n_current_rotation, n_current_x + 1, n_current_y)) ? 1 : 0;
    n_current_x -= ( b_key[1] && does_piece_fit(n_current_piece, n_current_rotation, n_current_x - 1, n_current_y)) ? 1 : 0;
    n_current_y += ( b_key[2] && does_piece_fit(n_current_piece, n_current_rotation, n_current_x, n_current_y + 1)) ? 1 : 0;

    //n_current_rotation += (b_key[3] && does_piece_fit(n_current_piece, n_current_rotation + 1,n_current_x, n_current_y)) ? 1 : 0;

    if(b_key[3]){
       n_current_rotation += (!b_rotate_hold && does_piece_fit(n_current_piece, n_current_rotation + 1,n_current_x, n_current_y)) ? 1 : 0;
       b_rotate_hold = true;
    }else{
        b_rotate_hold = false;
    }

    if(b_force_down){
        if(does_piece_fit(n_current_piece,n_current_rotation,n_current_x,n_current_y+1))
            n_current_y++; // If piece can go down take it down.
        else{
            // lock the piece into the field
            for(int px = 0; px < 4; px++)
                for(int py = 0; py < 4; py++)
                    if(tetromino[n_current_piece][Rotate(px, py, n_current_rotation)] == L'X')
                        p_field[(n_current_y + py)*n_field_width + (n_current_x + px)] = n_current_piece + 1; // fail on first hit

            n_piece_count++; // difficulty
            if(n_piece_count % 10 == 0)
                if(n_speed >= 10) n_speed--;

            // check if we have any lines
            for(int py =0; py < 4; py++){
                if(n_current_y +py < n_field_height - 1){
                    bool b_line = true;
                    for(int px = 1; px < n_field_width - 1; px++){
                        b_line &= (p_field[(n_current_y + py) * n_field_width + px]) != 0;
                    }
                    if(b_line){
                        // remove line, briefly set to =
                        for(int px = 1; px < n_field_width - 1; px++){
                            p_field[(n_current_y+py)*n_field_width + px] = 8;
                        }
                        v_lines.push_back(n_current_y + py); // store line height
                    }
                }
            }

            n_score += 25;
            if(!v_lines.empty()) n_score += (1 << v_lines.size()) * 100;

            // Choose the next piece
            n_current_piece = rand() % 7 ;
            n_current_rotation = 0;
            n_current_x = n_field_width / 2;
            n_current_y = 0;

            // if piece does not fit, then lost!
            b_game_over = !does_piece_fit(n_current_piece,n_current_rotation,n_current_x,n_current_y);
        }
        n_speed_counter = 0;

    }

    // Render Output

    // Draw Field
    for (int x = 0; x < n_field_width; x++)
    {
      for (int y = 0; y < n_field_height; y++)
      {
        screen[(y+2)*n_screen_width+(x+2)] = L" ABCDEFG=#"[p_field[y*n_field_width +x]];
      }
    }

    // -- draw current piece
    for(int px = 0; px < 4; px++)
        for(int py = 0; py < 4; py++)
            if(tetromino[n_current_piece][Rotate(px, py, n_current_rotation)] == L'X')
                screen[(n_current_y + py + 2)*n_screen_width + (n_current_x + px +2)] = n_current_piece + 65;

    // Draw Score
    swprintf_s(&screen[2* n_screen_width + n_field_width + 6], 16, L"SCORE: %8d", n_score);

    if(!v_lines.empty()){
        // Display frame
        WriteConsoleOutputCharacter(h_console, screen, n_screen_width * n_screen_height, {0,0}, &dw_bytes_written);
        this_thread::sleep_for(std::chrono::milliseconds(400));

        for(auto &v : v_lines){
            for(int px = 1; px < n_field_width - 1; px++){
                for(int py = v; py > 0; py--){ // remeber py-- actually goes up the rows above
                    p_field[py * n_field_width + px] = p_field[(py-1)*n_field_width+px];
                }
                p_field[px] = 0;
            }
        }
        v_lines.clear();
    }

    // Display frame
    WriteConsoleOutputCharacter(h_console, screen, n_screen_width * n_screen_height, {0,0}, &dw_bytes_written);

  }

  // Game Over :(
  CloseHandle(h_console);
  cout << "Game Over!! Score: "<< n_score << endl;
  system("pause");

  return 0;
}

/**
  Playing field dimensions from gameboy
  Store elements of the field as an array of unsigned chars.
  Dynamically not statically.

  this way: all map info stored in this array
   0 means empty spaces
   1 means part of the shape
   2 means different shape
   ...
   9 means boundary walls

**/

void create_assets(){
  tetromino[0].append(L"..X.");
  tetromino[0].append(L"..X.");
  tetromino[0].append(L"..X.");
  tetromino[0].append(L"..X.");

  tetromino[1].append(L"..X.");
  tetromino[1].append(L".XX.");
  tetromino[1].append(L".X..");
  tetromino[1].append(L"....");

  tetromino[2].append(L".X..");
  tetromino[2].append(L".XX.");
  tetromino[2].append(L"..X.");
  tetromino[2].append(L"....");

  tetromino[3].append(L"....");
  tetromino[3].append(L".XX.");
  tetromino[3].append(L".XX.");
  tetromino[3].append(L"....");

  tetromino[4].append(L"..X.");
  tetromino[4].append(L"..X.");
  tetromino[4].append(L".XX.");
  tetromino[4].append(L"....");

  tetromino[5].append(L"....");
  tetromino[5].append(L".XX.");
  tetromino[5].append(L"..X.");
  tetromino[5].append(L"..X.");

  tetromino[6].append(L"....");
  tetromino[6].append(L".XX.");
  tetromino[6].append(L".X..");
  tetromino[6].append(L".X..");
}

// Fix initial screen size -- effectively triggering word wrap
void resize_screen(){
    // Fix Screen Sizing
    _COORD coord;
    coord.X = 80;
    coord.Y = 34;

    _SMALL_RECT Rect;
    Rect.Top = 0;
    Rect.Left = 0;
    Rect.Bottom = 30 -5;
    Rect.Right = 80 -1;

    SetConsoleScreenBufferSize(h_console,coord);
    SetConsoleWindowInfo(h_console, TRUE, &Rect);

    Rect.Top = 0;
    Rect.Left = 0;
    Rect.Bottom = 30 +3;
    Rect.Right = 80 -1;

    SetConsoleScreenBufferSize(h_console,coord);
    SetConsoleWindowInfo(h_console, TRUE, &Rect);
    // Fix Screen Sizing End In short Causing a resize
}

/**
takes in x and y assuming 4x4 grid for tetrimino
r = 0, 1, or 2 or 0, 90, 180, or 270 rotation

**/
int Rotate(int px, int py, int r){
  switch( r % 4){
    case 0: return py * 4 + px; // 0 degress
    case 1: return 12 + py - (px * 4);  // 90 degrees
    case 2: return 15 - ( py * 4) -px; // 180 degrees
    case 3: return 3 - py + (px * 4); // 270 degrees
  }
  return 0;
}

/**
initialize array for the playing field.

set everything in the array to 0 unless,
its on the side of the array or at the bottom.

thus 9 will represent the boarder

**/
void create_playing_field(){
  p_field = new unsigned char[n_field_width * n_field_height]; // create play field buffer
  for(int x = 0; x < n_field_width; x++){ // Board Boundary
    for (int y = 0; y < n_field_height; y++)
    {
      p_field[y*n_field_width + x] = (x == 0 || x == n_field_width - 1 || y == n_field_height - 1) ? 9 : 0;
    }
  }
}

/**
effectively use the command line as a screen buffer
**/

void init_command_line_screen(){
  screen = new wchar_t[n_screen_width*n_screen_height];
  for (int i = 0; i < n_screen_width*n_screen_height; i++) screen[i] = L' ';
  h_console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
  SetConsoleActiveScreenBuffer(h_console); // This is default out instead of cout

  dw_bytes_written = 0;

  // Display Frame: and now we need to use a seperate command to draw to the buffer
  //WriteConsoleOutputCharacter(h_console, screen, n_screen_width * n_screen_height, {0,0}, &dw_bytes_written);

}

bool does_piece_fit(int n_tetromino, int n_rotation, int n_pos_x, int n_pos_y){
    // iterating over every piece of current tetris object
    for(int px = 0; px < 4; px++){
        for(int py = 0; py < 4; py++){

            // Get index into piece
            int pi = Rotate(px, py, n_rotation);

            // Get index into field
            int fi = (n_pos_y + py) * n_field_width + (n_pos_x +px);

            if(n_pos_x + px >= 0 && n_pos_x +px < n_field_width){
               if(n_pos_y + py >= 0 && n_pos_y + py < n_field_height){
                if(tetromino[n_tetromino][pi] == L'X' && p_field[fi] != 0)
                    return false; // fail on first hit
               }
            }
        }
    }
    return true;
}

// FAQ

/**

     Draw Field:
     Represents the whole board: the boandaries, empty space and tetris pieces

     notice string liter mapping:
         0: '' empty space
         1-7: gameboard A-G
         8: = later when completing a row
         9: # boarder of the game
    **/
/**

'L' means wchar_t, which, as opposed to a normal character,
requires 16-bits of storage rather than 8-bits. Here's an example:

"A"    = 41
"ABC"  = 41 42 43
L"A"   = 00 41
L"ABC" = 00 41 00 42 00 43

A wchar_t is twice big as a simple char. In daily use you don't need to use wchar_t,
but if you are using windows.h you are going to need it.


**/
