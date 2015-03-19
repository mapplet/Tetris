//The headers
#include "SDL/SDL.h"
#include "SDL_image/SDL_image.h"
#include <string>
#include <vector>
#include <queue>
#include "SDL_ttf/SDL_ttf.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <utility>

using namespace std;

//The attributes of the screen
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int SCREEN_BPP = 32;
const int BOARD_WIDTH = 10;
const int BOARD_HEIGHT = 24; //4 "invisible" rows at the top, where objects spawn, included
const int BLOCK_SIZE = 20; //One block = 20x20 px
const int BOARD_XPOS = 220; //Gameboard is placed 220px from left
const int BOARD_YPOS = -40; //Gameboard actually starts 40px on top of the screen

//The different object-surfaces that will be used
SDL_Surface* blockI = NULL;
SDL_Surface* blockJ = NULL;
SDL_Surface* blockL = NULL;
SDL_Surface* blockO = NULL;
SDL_Surface* blockS = NULL;
SDL_Surface* blockT = NULL;
SDL_Surface* blockZ = NULL;
SDL_Surface* edge = NULL;

//Other surfaces
SDL_Surface* background = NULL;
SDL_Surface* background_menu = NULL;
SDL_Surface* background_hs = NULL;
SDL_Surface* transparent = NULL;
SDL_Surface* play_button = NULL;
SDL_Surface* play_marked = NULL;
SDL_Surface* highscore_button = NULL;
SDL_Surface* highscore_marked = NULL;
SDL_Surface* quit_button = NULL;
SDL_Surface* quit_marked = NULL;


SDL_Surface* screen = NULL;
SDL_Surface* score_message = NULL;
SDL_Surface* level_message = NULL;
SDL_Surface* highscore_candidate = NULL;
SDL_Surface* name = NULL;

SDL_Event event;
TTF_Font* font = NULL;
SDL_Color textColor = { 255, 255, 255 };

//Functions and classes
SDL_Surface *load_image( std::string filename )
{
    //Temporary storage for the image that's loaded
    SDL_Surface* loadedImage = NULL;
    
    //The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;
    
    //Load the image
    loadedImage = IMG_Load( filename.c_str() );
    
    //If nothing went wrong in loading the image
    if( loadedImage != NULL )
    {
        //Create an optimized image
        optimizedImage = SDL_DisplayFormatAlpha( loadedImage );
        
        //Free the old image
        SDL_FreeSurface( loadedImage );
    }
    
    //Return the optimized image
    return optimizedImage;
}

class Tetris
{
public:
    Tetris() {}
    
    void apply_surface(int, int, SDL_Surface*);
    void apply_board(int, int, int, int, SDL_Surface*);
    
};

void Tetris::apply_surface(int x, int y, SDL_Surface* source)
{
    //Make a temporary rectangle to hold the offsets
    SDL_Rect offset;
    
    //Give the offsets to the rectangle
    offset.x = x;
    offset.y = y;
    
    //Blit the surface
    SDL_BlitSurface(source, NULL, screen, &offset);
}

void Tetris::apply_board(int x, int y, int cW, int cH, SDL_Surface* source)
{
    //Make a temporary rectangle to hold the offsets
    SDL_Rect offset;
    
    //Give the offsets to the rectangle
    offset.x = x;
    offset.y = y;
    
    SDL_Rect crop;
    crop.x = x;
    crop.y = y;
    crop.w = cW;
    crop.h = cH;
    
    //Blit the surface
    SDL_BlitSurface(source, &crop, screen, &offset);
}


class Object : public Tetris
{
public:
    Object(Uint8 type)
    : type_(type), xPos_(3), yPos_(0), exchanged(false) { set_matrix_type(); }
    
    //Get-functions
    Uint8 get_type() const { return type_; }
    int get_xPos() const { return xPos_; } //Returns x-pos for the 5x5-objects [0][0]-block
    int get_yPos() const { return yPos_; } //Returns y-pos for the 5x5-objects [0][0]-block

    //Sets and actions
    void set_xPos(int xPos) { xPos_ = xPos; }
    void set_yPos(int yPos) { yPos_ = yPos; }
    void set_exchanged() { exchanged = true; }
    void rotate_left();
    void rotate_right();
    
    //Draw functions
    void draw_object();
    void draw_saved_object();
    void draw_next();
    void draw_predicted_position();
    
    bool isExchanged() { return exchanged; }
    
    //Variables
    int matrix_[5][5]; //5x5 matrix to build the object of
    
private:
    Uint8 type_; //1=I, 2=J, 3=L, 4=O, 5=S, 6=T, 7=Z
    int xPos_;
    int yPos_;
    bool exchanged;
    
    void set_matrix_type();
};

void Object::rotate_left()
{
    if (type_ != 4) //type 4 (O, square) does not rotate
    {
        queue<int> temp;
        
        for (int i=0; i<5; ++i)
        {
            for (int j=0; j<5; ++j)
            {
                temp.push(matrix_[j][i]);
            }
        }
        
        for (int i=0; i<5; ++i)
        {
            for (int j=4; j>=0; --j)
            {
                matrix_[i][j] = temp.front();
                temp.pop();
            }
        }
    }
}

void Object::rotate_right()
{
    if (type_ != 4) //type 4 (O, square) does not rotate
    {
        queue<int> temp;
        
        for (int i=0; i<5; ++i)
        {
            for (int j=0; j<5; ++j)
            {
                temp.push(matrix_[j][i]);
            }
        }
        
        for (int i=4; i>=0; --i)
        {
            for (int j=0; j<5; ++j)
            {
                matrix_[i][j] = temp.front();
                temp.pop();
            }
        }
    }
}

void Object::set_matrix_type()
{
    
    for (int x=0; x<5; ++x)
    {
        for (int y=0; y<5; ++y)
            matrix_[x][y] = 0;
    }
    
    if (type_ == 1) // I
    {
        matrix_[2][0] = 1;
        matrix_[2][1] = 1;
        matrix_[2][2] = 1;
        matrix_[2][3] = 1;
    }
    
    if (type_ == 2) // J
    {
        matrix_[1][3] = 2;
        matrix_[2][1] = 2;
        matrix_[2][2] = 2;
        matrix_[2][3] = 2;
    }
    
    if (type_ == 3) // L
    {
        matrix_[2][1] = 3;
        matrix_[2][2] = 3;
        matrix_[2][3] = 3;
        matrix_[3][3] = 3;
    }
    
    if (type_ == 4) // O
    {
        matrix_[1][1] = 4;
        matrix_[1][2] = 4;
        matrix_[2][1] = 4;
        matrix_[2][2] = 4;
    }
    
    if (type_ == 5) // S
    {
        matrix_[1][1] = 5;
        matrix_[1][2] = 5;
        matrix_[2][2] = 5;
        matrix_[2][3] = 5;
    }
    
    if (type_ == 6) // T
    {
        matrix_[2][1] = 6;
        matrix_[1][2] = 6;
        matrix_[2][2] = 6;
        matrix_[3][2] = 6;
    }
    
    if (type_ == 7) // Z
    {
        matrix_[2][1] = 7;
        matrix_[1][2] = 7;
        matrix_[2][2] = 7;
        matrix_[1][3] = 7;
    }
}

void Object::draw_object()
{
    vector<SDL_Surface*> blockvector {blockI, blockJ, blockL, blockO, blockS, blockT, blockZ};
    
    for (int i=0; i<5; ++i)
    {
        for (int j=0; j<5; ++j)
        {
            if (matrix_[i][j] != 0)
            {
                apply_surface(BOARD_XPOS+(get_xPos()*BLOCK_SIZE)+(i*BLOCK_SIZE), BOARD_YPOS+(get_yPos()*BLOCK_SIZE)+(j*BLOCK_SIZE), blockvector.at(matrix_[i][j] -1));
            }
        }
    }
    apply_board(220, 0, 200, 40, background);
}

void Object::draw_next()
{
    vector<SDL_Surface*> blockvector {blockI, blockJ, blockL, blockO, blockS, blockT, blockZ};
    
    apply_board(420, 0, BOARD_XPOS, 140, background);
    for (int i=0; i<5; ++i)
    {
        for (int j=0; j<5; ++j)
        {
            if (matrix_[i][j] != 0)
            {
                apply_surface(440+(i*BLOCK_SIZE), 40+(j*BLOCK_SIZE), blockvector.at(matrix_[i][j] -1));
            }
        }
    }
}

void Object::draw_saved_object()
{
    vector<SDL_Surface*> blockvector {blockI, blockJ, blockL, blockO, blockS, blockT, blockZ};
    
    apply_board(0, 0, BOARD_XPOS, SCREEN_HEIGHT, background);
    for (int i=0; i<5; ++i)
    {
        for (int j=0; j<5; ++j)
        {
            if (matrix_[i][j] != 0)
            {
                apply_surface(100+(i*BLOCK_SIZE), 40+(j*BLOCK_SIZE), blockvector.at(matrix_[i][j] -1));
            }
        }
    }
}

void Object::draw_predicted_position()
{
    for (int i=0; i<5; ++i)
    {
        for (int j=0; j<5; ++j)
        {
            if (matrix_[i][j] != 0)
            {
                apply_surface(BOARD_XPOS+(get_xPos()*BLOCK_SIZE)+(i*BLOCK_SIZE), BOARD_YPOS+(get_yPos()*BLOCK_SIZE)+(j*BLOCK_SIZE), edge);
            }
        }
    }
    apply_board(220, 0, 200, 40, background);
}


class Board : public Tetris
{
public:
    Board()
    : score_(0), level_(1) { init_boardMatrix(); }
    
    void init_boardMatrix();
    void draw_board();
    bool isMovementPossible(const Object&); //Returns false if we've done something illegal
    void store_object(Object&);
    void clear_row(Object&); //Returns the sum of rows cleared at the same time
    void drop_blocks(int&); //Moves all the stored blocks from y=0 to y=int&argument down to fill cleared rows
    bool isGameover(Object&);
    void increase_score(int&);
    void increase_level() { ++level_; }
    int get_score() { return score_; }
    
    void print_score_level();
    
private:
    int boardMatrix[BOARD_WIDTH][BOARD_HEIGHT];
    int score_;
    int level_;
};

void Board::init_boardMatrix()
{
    for (int i=0; i<BOARD_WIDTH; ++i)
    {
        for (int j=0; j<BOARD_HEIGHT; ++j)
            boardMatrix[i][j] = 0;
    }
}

void Board::draw_board()
{
    vector<SDL_Surface*> blockvector {blockI, blockJ, blockL, blockO, blockS, blockT, blockZ};
    
    for (int y=4; y<BOARD_HEIGHT; ++y)
    {
        for (int x=0; x<BOARD_WIDTH; ++x)
        {
            if (boardMatrix[x][y] != 0)
            {
                apply_surface(BOARD_XPOS+(x*BLOCK_SIZE), BOARD_YPOS+(y*BLOCK_SIZE), blockvector.at(boardMatrix[x][y] -1));
            }
            else
                apply_board(BOARD_XPOS+(x*BLOCK_SIZE), BOARD_YPOS+(y*BLOCK_SIZE), 20, 20, background);
        }
    }
}

bool Board::isMovementPossible(const Object& object)
{
    for (int i = object.get_xPos(); i<object.get_xPos()+5; ++i)
    {
        for (int j = object.get_yPos(); j<object.get_yPos()+5; ++j)
        {
            if (object.matrix_[i-object.get_xPos()][j-object.get_yPos()] != 0
                && (i < 0 || i >= BOARD_WIDTH || j >= BOARD_HEIGHT)) // If we are out of bounds
                return false;
        }
    }
     
    
    int x; // x-pos to loop from..
    int x_; // x-pos to loop to..
    int y; // y-pos to loop from..
    int y_; // y-pos to loop to..
    // ..to check if there is a block associated to the object that has collided with a block stored in the gameboard
    
    if (object.get_xPos() < 0) // If our 5x5 object is outside the left boarder of the gameboard
    {
        x = 0;
        x_ = object.get_xPos()+5;
    }
    else if (object.get_xPos() > 5) // If our 5x5 object is outside the right boarder of the gameboard
    {
        x = object.get_xPos();
        x_ = BOARD_WIDTH;
    }
    else
    {
        x = object.get_xPos();
        x_ = x+5;
    }

    if (object.get_yPos() > (BOARD_HEIGHT -5)) // If our 5x5 object is outside the bottom boarder of the gameboard
    {
        y = object.get_yPos();
        y_ = BOARD_HEIGHT;
    }
    else
    {
        y = object.get_yPos();
        y_ = y+5;
    }
    
    for (int i = x; i < x_; ++i)
    {
        for (int j = y; j < y_; ++j)
        {
            if ((boardMatrix[i][j] != 0 // If two blocks have collided
                 && object.matrix_[i-object.get_xPos()][j-object.get_yPos()] != 0))
                return false;
        }
    }

    return true;
}

void Board::store_object(Object& current)
{
    for (int x = current.get_xPos(); x<current.get_xPos()+5; ++x)
    {
        for (int y = current.get_yPos(); y<current.get_yPos()+5; ++y)
        {
            if (current.matrix_[x-current.get_xPos()][y-current.get_yPos()] != 0)
                boardMatrix[x][y] = current.matrix_[x-current.get_xPos()][y-current.get_yPos()];
        }
    }
}

void Board::clear_row(Object& current)
{
    int rows_cleared = 0;
    
    for (int y=current.get_yPos(); y<(current.get_yPos()+5); ++y) // Loop through possible rows in y-direction to clear
    {
        for (int x=0; x<BOARD_WIDTH; ++x) // Loop through the whole row in x-direction
        {
            if (boardMatrix[x][y] == 0)
                break;
            if (x == 9) // If we looped through and the row was empty on blocks
            {
                drop_blocks(y); // Move down all the overlying blocks
                ++rows_cleared;
            }
        }
    }
    
    increase_score(rows_cleared);
}

void Board::drop_blocks(int& y_init)
{
    for (int y = y_init; y>0; --y) // Move all stored blocks from the top to y_init, in y-direction, down one step
    {
        for (int x=0; x<BOARD_WIDTH; ++x)
        {
            boardMatrix[x][y] = boardMatrix[x][y-1];
        }
    }
}

bool Board::isGameover(Object& current)
{
    if (!isMovementPossible(current))
        return true;
    return false;
}

void Board::increase_score(int& rows)
{
    if (rows == 1)
        score_ += 100;
    else if(rows == 2)
        score_ += 250;
    else if (rows == 3)
        score_ += 400;
    else if (rows == 4)
        score_ += 550;
}

void Board::print_score_level()
{
    stringstream ss;
    stringstream ss2; //Vet ej varför det inte går att återanvända ss ??
    char score[10];
    char level[3];

    ss << score_;
    ss >> score;
    ss2 << level_;
    ss2 >> level;
    
    score_message = TTF_RenderText_Solid(font, score, textColor);
    level_message = TTF_RenderText_Solid(font, level, textColor);
    
    apply_board(440, 222, 200, 258, background);
    apply_surface(440, 222, score_message);
    apply_surface(440, 325, level_message);
}


bool init()
{
    //Initialize all SDL subsystems
    if( SDL_Init(SDL_INIT_EVERYTHING) == -1 )
    {
        return false;
    }
    
    //Set up the screen
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
    
    //If there was an error in setting up the screen
    if( screen == NULL )
    {
        return false;
    }
    
    if (TTF_Init() == -1)
    {
        return false;
    }
    
    //Set the window caption
    SDL_WM_SetCaption( "Tetris", NULL );
    
    //If everything initialized fine
    return true;
}

bool load_files()
{
    //Load the images
    
    blockI = load_image("Images/Blocks/I_Blue.png");
    blockJ = load_image("Images/Blocks/J_Pink.png");
    blockL = load_image("Images/Blocks/L_Bronze.png");
    blockO = load_image("Images/Blocks/O_Red.png");
    blockS = load_image("Images/Blocks/S_Yellow.png");
    blockT = load_image("Images/Blocks/T_Orange.png");
    blockZ = load_image("Images/Blocks/Z_Green.png");
    edge = load_image("Images/Blocks/Edge.png");
    background = load_image("Images/Background.png" );
    background_menu = load_image("Images/Menu.png" );
    background_hs = load_image("Images/Highscore_bg.png" );
    transparent = load_image("Images/Transparent_Enter.png");
    
    play_button = load_image("Images/Buttons/Play.png");
    play_marked = load_image("Images/Buttons/Play2.png");
    highscore_button = load_image("Images/Buttons/Highscore.png");
    highscore_marked = load_image("Images/Buttons/Highscore2.png");
    quit_button = load_image("Images/Buttons/Quit.png");
    quit_marked = load_image("Images/Buttons/Quit2.png");
    
    font = TTF_OpenFont("DrawingPad.ttf", 28 );
    
    
    //If there was an error in loading the image
    if( background == NULL || blockI == NULL || blockJ == NULL || blockL == NULL || blockO == NULL || blockS == NULL || blockT == NULL || blockZ == NULL || font == NULL)
    {
        return false;
    }
    
    //If everything loaded fine
    return true;
}

void clean_up()
{
    //Free the images
    SDL_FreeSurface(background);
    SDL_FreeSurface(blockI);
    SDL_FreeSurface(blockJ);
    SDL_FreeSurface(blockL);
    SDL_FreeSurface(blockO);
    SDL_FreeSurface(blockS);
    SDL_FreeSurface(blockT);
    SDL_FreeSurface(blockZ);
    SDL_FreeSurface(edge);
    SDL_FreeSurface(score_message);
    SDL_FreeSurface(level_message);
    SDL_FreeSurface(play_button);
    SDL_FreeSurface(play_marked);
    SDL_FreeSurface(highscore_button);
    SDL_FreeSurface(highscore_marked);
    SDL_FreeSurface(quit_button);
    SDL_FreeSurface(quit_marked);
    SDL_FreeSurface(background_hs);
    SDL_FreeSurface(background_menu);
    SDL_FreeSurface(transparent);
    //SDL_FreeSurface(highscore_candidate);
    
    //Quit SDL
    SDL_Quit();
}

int get_new_random(Object& current)
{
    int next_type = (SDL_GetTicks() % 7) +1;
    
    while(next_type == current.get_type())
    {
        next_type = (SDL_GetTicks() % 7) +1;
    }

    return next_type;
}

void update_predicted_position(Object& predicted_position, Object& current, Board& board)
{
    predicted_position = current;
    while (board.isMovementPossible(predicted_position))
    {
        predicted_position.set_yPos(predicted_position.get_yPos() +1);
    }
    predicted_position.set_yPos(predicted_position.get_yPos() -1);
}

void view_menu(bool& quit, string& state)
{
    Tetris tetris;
    tetris.apply_surface(0, 0, background_menu);
    SDL_Flip(screen);
    Uint8 menu_state = 0;
    
    bool leave_state = false;
    
    while(!leave_state)
    {
        while(SDL_PollEvent(&event))
        {
            
            if (menu_state == 0)
            {
                tetris.apply_surface(0, 0, background_menu);
                tetris.apply_surface(205, 150, play_marked);
                tetris.apply_surface(205, 150+64, highscore_button);
                tetris.apply_surface(205, 150+(64*2), quit_button);
                SDL_Flip(screen);
            }
            else if (menu_state == 1)
            {
                tetris.apply_surface(0, 0, background_menu);
                tetris.apply_surface(205, 150, play_button);
                tetris.apply_surface(205, 150+64, highscore_marked);
                tetris.apply_surface(205, 150+(64*2), quit_button);
                SDL_Flip(screen);
            }
            else if (menu_state == 2)
            {
                tetris.apply_surface(0, 0, background_menu);
                tetris.apply_surface(205, 150, play_button);
                tetris.apply_surface(205, 150+64, highscore_button);
                tetris.apply_surface(205, 150+(64*2), quit_marked);
                SDL_Flip(screen);
            }
            
            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                leave_state = true;
                //Quit the program
                quit = true;
            }
            
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_DOWN && menu_state < 2)
                    ++menu_state;
                
                if (event.key.keysym.sym == SDLK_UP && menu_state > 0)
                    --menu_state;
                
                if (event.key.keysym.sym == SDLK_RETURN)
                {
                    if (menu_state == 0)
                    {
                        leave_state = true;
                        state = "PLAY";
                    }
                    else if (menu_state == 1)
                    {
                        leave_state = true;
                        state = "HIGHSCORE";
                    }
                    else if (menu_state == 2)
                    {
                        leave_state = true;
                        quit = true;
                    }
                }
            }
            
        }
        
    }

}

int run_game(bool& quit, string& state)
{
    bool leave_state = false;
    //Bool to check if there is a saved object
    bool saved_object_exist = false;
    
    Tetris tetris; //Create main-class
    Object current(get_new_random(current)); //Create our current Tetris-object
    Object next = current; //Create next Tetris-object
    if (next.get_type() < 7)
    {
        Object temp(get_new_random(temp) +1);
        next = temp;
    }
    else
    {
        Object temp(get_new_random(temp) -1);
        next = temp;
    }
    
    Object saved_object(get_new_random(current));
    Object predicted_position(current.get_type());
    Board board; //Create the gameboard
    
    //For level increasement
    int speed = 800;
    int objects = 1; //Counts

    //Apply the background to the screen
    tetris.apply_surface( 0, 0, background );
    
    //Update the screen
    SDL_Flip(screen);
    
    
    Uint32 time = SDL_GetTicks();
    
    //While the user hasn't quit
    while(!leave_state)
    {
        
        if (objects == 20)
        {
            if (speed != 125)
                speed -= 75;
            objects = 0;
            board.increase_level();
        }
        
        
        next.draw_next();
        
        if(saved_object_exist)
            saved_object.draw_saved_object();
        
        //Let the piece fall down after xx ms
        if ((SDL_GetTicks() - time) >= speed)
        {
            current.set_yPos(current.get_yPos() +1);
            if (!board.isMovementPossible(current))
            {
                current.set_yPos(current.get_yPos() -1);
                board.store_object(current);
                board.clear_row(current);
                current = next;
                ++objects;
                next = Object(get_new_random(next));
                if (board.isGameover(current))
                {
                    leave_state = true;
                    state = "GAME OVER";
                }
            }
            
            update_predicted_position(predicted_position, current, board);
            board.draw_board();
            current.draw_object();
            predicted_position.draw_predicted_position();
            time = SDL_GetTicks();
            
            board.print_score_level();
            
            SDL_Flip(screen);
 
        }
        //While there's an event to handle
        while(SDL_PollEvent(&event))
        {
            //If the user has Xed out the window
            if( event.type == SDL_QUIT )
            {
                leave_state = true;
                //Quit the program
                quit = true;
            }
            
            //If a key was pressed
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    state = "MENU";
                    leave_state = true;
                }
                
                if (event.key.keysym.sym == SDLK_DOWN)
                {
                    current.set_yPos(current.get_yPos() + 1);
                    if (!board.isMovementPossible(current))
                    {
                        current.set_yPos(current.get_yPos() -1);
                    }
                    update_predicted_position(predicted_position, current, board);
                    board.draw_board();
                    current.draw_object();
                    predicted_position.draw_predicted_position();
                }
                
                if (event.key.keysym.sym == SDLK_UP)
                {
                    current.rotate_left();
                    if (!board.isMovementPossible(current))
                    {
                        current.rotate_right();
                    }
                    update_predicted_position(predicted_position, current, board);
                    board.draw_board();
                    current.draw_object();
                    predicted_position.draw_predicted_position();
                }
                
                if (event.key.keysym.sym == SDLK_RIGHT)
                {
                    current.set_xPos(current.get_xPos() + 1);
                    if (!board.isMovementPossible(current))
                    {
                        current.set_xPos(current.get_xPos() -1);
                    }
                    update_predicted_position(predicted_position, current, board);
                    board.draw_board();
                    current.draw_object();
                    predicted_position.draw_predicted_position();
                }
                
                if (event.key.keysym.sym == SDLK_LEFT)
                {
                    current.set_xPos(current.get_xPos() - 1);
                    if (!board.isMovementPossible(current))
                    {
                        current.set_xPos(current.get_xPos() +1);
                    }
                    update_predicted_position(predicted_position, current, board);
                    board.draw_board();
                    current.draw_object();
                    predicted_position.draw_predicted_position();
                }
                
                if (event.key.keysym.sym == SDLK_z)
                {
                    current.rotate_left();
                    if (!board.isMovementPossible(current))
                    {
                        current.rotate_right();
                    }
                    update_predicted_position(predicted_position, current, board);
                    board.draw_board();
                    current.draw_object();
                    predicted_position.draw_predicted_position();
                }
                
                if (event.key.keysym.sym == SDLK_x)
                {
                    current.rotate_right();
                    if (!board.isMovementPossible(current))
                    {
                        current.rotate_left();
                    }
                    update_predicted_position(predicted_position, current, board);
                    board.draw_board();
                    current.draw_object();
                    predicted_position.draw_predicted_position();
                }
                
                if (event.key.keysym.sym == SDLK_SPACE)
                {
                    while (board.isMovementPossible(current))
                    {
                        current.set_yPos((current.get_yPos()+1));
                    }
                    current.set_yPos(current.get_yPos() -1);
                    
                    board.store_object(current);
                    board.clear_row(current);
                    current = next;
                    ++objects;
                    next = Object(get_new_random(next));
                    
                    board.draw_board();
                }
                
                if (event.key.keysym.sym == SDLK_LSHIFT && !current.isExchanged())
                {
                    Object temp = saved_object;
                    if(saved_object_exist == false)
                    {
                        saved_object = current;
                        current = next;
                        next = Object(get_new_random(next));
                        saved_object_exist = true;
                    }
                    else
                    {
                        saved_object = current;
                        temp.set_xPos(3);
                        temp.set_yPos(0);
                        current = temp;
                    }
                    current.set_exchanged();
                    saved_object.draw_saved_object();
                    board.draw_board();
                }
            }
            
            SDL_Flip(screen);
            
        }
        
    }
    return board.get_score();
}

bool sortFunction(pair<string,int> i,pair<string, int> j)
{
    if(j.second > i.second)
        return false;
    return true;
}

void view_highscore(bool& quit, string& state, vector< pair<string, int>>& score_vector)
{
    Tetris tetris;
    tetris.apply_surface(0, 0, background_hs);
    SDL_Flip(screen);
    bool leave_state = false;
    
    score_vector.clear();
    
    string name;
    int score;
    
    ifstream file("Highscore.txt");
    
    while(file >> name >> score)
    {
        score_vector.push_back(make_pair(name, score));
    }
    sort(score_vector.begin(), score_vector.end(), sortFunction);
    file.close();
    
    
    int Y = 120;
    string str_;
    stringstream ss;
    for(int i = 0; i < score_vector.size(); ++i)
    {
        ss << score_vector.at(i).first;
        
        str_ = ss.str();
        ss.str("");
        highscore_candidate = TTF_RenderText_Solid(font, str_.c_str(), textColor);
        tetris.apply_surface(200, Y, highscore_candidate);
        SDL_Flip(screen);
        SDL_FreeSurface(highscore_candidate);
        
        ss << score_vector.at(i).second;
        str_ = ss.str();
        ss.str("");
        
        highscore_candidate = TTF_RenderText_Solid(font, str_.c_str(), textColor);
        tetris.apply_surface(350, Y, highscore_candidate);
        SDL_Flip(screen);
        SDL_FreeSurface(highscore_candidate);
        Y += 25;
    }
    
    //Placeringsiffran
    Y = 120;
    for(int i = 1; i<=10; ++i)
    {
        ss << i << '.';
        str_ = ss.str();
        ss.str("");
        
        highscore_candidate = TTF_RenderText_Solid(font, str_.c_str(), textColor);
        tetris.apply_surface(160, Y, highscore_candidate);
        SDL_Flip(screen);
        SDL_FreeSurface(highscore_candidate);
        Y += 25;
    }
    
    while(!leave_state)
    {
        while(SDL_PollEvent(&event))
        {
            if( event.type == SDL_QUIT )
            {
                leave_state = true;
                //Quit the program
                quit = true;
            }
            
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_ESCAPE)
                {
                    leave_state = true;
                    state = "MENU";
                }
            }
        }
        
    }
}

void save_highscore(bool& quit, string& state, int& score, vector< pair<string, int>>& score_vector)
{
    Tetris tetris;
    tetris.apply_surface(0, 0, transparent);
    SDL_Flip(screen);
    
    bool leave_state = false;
    bool name_entered = false;
    string name_str = "";
    SDL_EnableUNICODE(SDL_ENABLE);
    
    while(!leave_state)
    {
        while (!name_entered)
        {
            while(SDL_PollEvent(&event))
            {
                if( event.type == SDL_QUIT )
                {
                    name_entered = true;
                    leave_state = true;
                    //Quit the program
                    quit = true;
                }
            
                if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_ESCAPE) //If you don't want to save
                    {
                        name_entered = true;
                        leave_state = true;
                        state = "MENU";
                    }
                    
                    string temp_copy = name_str;
                    
                    //If the string less than maximum size
                    if(name_str.length() <= 16)
                    {
                        //If the key is a space
                        if(event.key.keysym.unicode == (Uint16)' ')
                        {
                            //Append the character
                            name_str += (char)event.key.keysym.unicode;
                        }
                        //If the key is a number
                        else if( ( event.key.keysym.unicode >= (Uint16)'0' ) && ( event.key.keysym.unicode <= (Uint16)'9' ) )
                        {
                            //Append the character
                            name_str += (char)event.key.keysym.unicode;
                        }
                        //If the key is a uppercase letter
                        else if( ( event.key.keysym.unicode >= (Uint16)'A' ) && ( event.key.keysym.unicode <= (Uint16)'Z' ) )
                        {
                            //Append the character
                            name_str += (char)event.key.keysym.unicode;
                        }
                        //If the key is a lowercase letter
                        else if( ( event.key.keysym.unicode >= (Uint16)'a' ) && ( event.key.keysym.unicode <= (Uint16)'z' ) )
                        {
                            //Append the character
                            name_str += (char)event.key.keysym.unicode;
                        }
                    }
                    
                    //If backspace was pressed and the string isn't blank
                    if ((event.key.keysym.sym == SDLK_BACKSPACE) && (name_str.length() != 0))
                    {
                        //Remove a character from the end
                        name_str.erase(name_str.length() - 1);
                    }
                    
                    //If the string was changed
                    if (name_str != temp_copy)
                    {
                        //Free the old surface
                        SDL_FreeSurface(name);
                        
                        //Render a new text surface
                        name = TTF_RenderText_Solid(font, name_str.c_str(), textColor);
                    }
                }
            }
            
            //If the surface isn't blank
            if( name != NULL )
            {
                tetris.apply_surface(0, 0, background);
                tetris.apply_surface(0, 0, transparent);
                //Show the name
                tetris.apply_surface((SCREEN_WIDTH - name->w)/2, (SCREEN_HEIGHT - name->h)/2, name);
                SDL_Flip(screen);
            }
            
            //If the enter key was pressed
            if( ( event.type == SDL_KEYDOWN ) && ( event.key.keysym.sym == SDLK_RETURN ) )
            {
                //Free the old message surface
                SDL_FreeSurface(name);
                
                if(score_vector.size() < 10)
                {
                    score_vector.push_back(make_pair(name_str, score));
                }
                
                else
                {
                    score_vector.pop_back();
                    score_vector.push_back(make_pair(name_str, score));
                }
                
                sort(score_vector.begin(), score_vector.end(), sortFunction);
                
                remove("Highscore.txt");
                fstream file;
                file.open("Highscore.txt", ios_base::in|ios_base::out|ios_base::trunc);
                
                
                for(int i = 0; i < score_vector.size(); ++i)
                {
                    file << score_vector.at(i).first;
                    file << " "; 
                    file << score_vector.at(i).second;
                    file << endl; 
                }
                
                file.close();
        
                
                //Change the flag
                name_entered = true;
                leave_state = true;
                state = "HIGHSCORE";
            }
        }
    }
}

void check_score(bool& quit, string& state, int& score, vector< pair<string, int>>& score_vector)
{
    if (score_vector.size() < 10 || score_vector.at(score_vector.size()-1).second < score)
        save_highscore(quit, state, score, score_vector);
    else
    {
    state = "MENU"; //visa gameover bara
    }
}

// Main
int main( int argc, char* args[] )
{
    //Make sure the program waits for a quit
    bool quit = false;
    
    //Initialize
    if( init() == false )
        return 1;
   
    //Load the files
    if(load_files() == false)
    {
        return 1;
    }
    
    string state = "MENU";
    int score;
    vector< pair<string, int>> score_vector;
    
    while (quit == false)
    {
        if (state == "MENU")
            view_menu(quit, state);
        if (state == "PLAY")
            score = run_game(quit, state);
        if (state == "HIGHSCORE")
            view_highscore(quit, state, score_vector);
        if (state == "GAME OVER")
            check_score(quit, state, score, score_vector);
    }
    
    //Free the surface and quit SDL
    clean_up();
    
    return 0;
}

