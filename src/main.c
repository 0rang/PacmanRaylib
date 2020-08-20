//am likely to be using this resource alot: https://gameinternals.com/understanding-pac-man-ghost-behavior

#include "math.h"
#include "raylib.h"
#include "raymath.h"

#define NDEBUG
#define SCREEN_WIDTH (672)
#define SCREEN_HEIGHT (864)
#define TILE_LENGTH (24)

#define TILES_IN_ROW (28)
#define TILES_IN_COLUMN (36)
#define NO_OF_TILES (TILES_IN_ROW*TILES_IN_COLUMN)
#define NUM_ACCESSIBLE_TILES (300)
#define NUM_JUNCTION_TILES (64)
#define TAC_POINTS (10)

#define PACMAN_SPEED (2)
#define BLINKY_SPEED (1)

#define BLINKY_START_TILE (Vector2Int){13, 14}

#define ASSET_PATH "../assets/"

typedef struct Vector2Int
{
    int x;
    int y;
} Vector2Int;

enum Direction
{
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NONE
};

enum TileType
{
    INACCESSIBLE = 0,
    STRAIGHT_PATH,
    JUNCTION
};

typedef struct JunctionTileInfo
{
    int x;
    int y;
    
    bool up_access;
    bool right_access;
    bool down_access;
    bool left_access;
    
    bool special_access;
} JunctionTileInfo;

typedef struct Tile
{
    enum TileType type;
    bool has_tictac;
    bool has_energiser;
    
    //coordinate of tile
    int x;
    int y;
    
    struct
    {
        bool up_access;
        bool right_access;
        bool down_access;
        bool left_access;
        
        bool special_access;
    } junction_info;
    
} Tile;

enum GhostMode{
    SCATTER = 0,
    CHASE,
    FRIGHTENED,
    EATEN
};

typedef struct Ghost{
    enum GhostMode mode;
    enum GhostMode mode_before_eaten;
    bool mode_switch; //so that direction can be reversed when mode is switched
    
    Vector2 centre_pos;
    enum Direction direction;
    
    Tile current_tile;
    bool move_choices[4];
    Tile previous_tile;
    Vector2Int scatter_target;
    Vector2Int current_target;
    
    float timer;
    float last_get_time;
    int mode_times_index;
    
}Ghost;

//TODO: special tacs add 50 points
int score = 0;
bool player_won = true;
int pacman_lives = 3;
Vector2 pacman_centre;
enum Direction pacman_direction;

Ghost blinky_r;
Ghost pinky_p;
Ghost inky_b;
Ghost clyde_o;

//values at even indices are scatter mode times, odd indices are chase mode times
float mode_times_lv1[8] = {7, 20, 7, 20, 5, 20, 5, INFINITY};

//make all tiles inaccessible
Tile tile_map[TILES_IN_ROW][TILES_IN_COLUMN] = {0};

void reset_tiles(){
    //Configure all the accessible tiles here. Include junction tiles. types will be corrected later.
    {
        Vector2Int accessible_tiles[NUM_ACCESSIBLE_TILES] = {
                {1,  4},
                {2,  4},
                {3,  4},
                {4,  4},
                {5,  4},
                {6,  4},
                {7,  4},
                {8,  4},
                {9,  4},
                {10, 4},
                {11, 4},
                {12, 4},
                {15, 4},
                {16, 4},
                {17, 4},
                {18, 4},
                {19, 4},
                {20, 4},
                {21, 4},
                {22, 4},
                {23, 4},
                {24, 4},
                {25, 4},
                {26, 4},
                {1,  5},
                {1,  6},
                {1,  7},
                {6,  5},
                {6,  6},
                {6,  7},
                {12, 5},
                {12, 6},
                {12, 7},
                {15, 5},
                {15, 6},
                {15, 7},
                {21, 5},
                {21, 6},
                {21, 7},
                {26, 5},
                {26, 6},
                {26, 7},
                {1,  8},
                {2,  8},
                {3,  8},
                {4,  8},
                {5,  8},
                {6,  8},
                {7,  8},
                {8,  8},
                {9,  8},
                {10, 8},
                {11, 8},
                {12, 8},
                {13, 8},
                {14, 8},
                {15, 8},
                {16, 8},
                {17, 8},
                {18, 8},
                {19, 8},
                {20, 8},
                {21, 8},
                {22, 8},
                {23, 8},
                {24, 8},
                {25, 8},
                {26, 8},
                {1,  9},
                {1,  10},
                {6,  9},
                {6,  10},
                {9,  9},
                {9,  10},
                {18, 9},
                {18, 10},
                {21, 9},
                {21, 10},
                {26, 9},
                {26, 10},
                {1,  11},
                {2,  11},
                {3,  11},
                {4,  11},
                {5,  11},
                {6,  11},
                {9,  11},
                {10, 11},
                {11, 11},
                {12, 11},
                {15, 11},
                {16, 11},
                {17, 11},
                {18, 11},
                {21, 11},
                {22, 11},
                {23, 11},
                {24, 11},
                {25, 11},
                {26, 11},
                {6,  12},
                {6,  13},
                {6,  14},
                {6,  15},
                {6,  16},
                {6,  17},
                {6,  18},
                {6,  19},
                {6,  20},
                {6,  21},
                {6,  22},
                {21, 12},
                {21, 13},
                {21, 14},
                {21, 15},
                {21, 16},
                {21, 17},
                {21, 18},
                {21, 19},
                {21, 20},
                {21, 21},
                {21, 22},
                {15, 12},
                {15, 13},
                {12, 12},
                {12, 13},
                {9,  14},
                {10, 14},
                {11, 14},
                {12, 14},
                {13, 14},
                {14, 14},
                {15, 14},
                {16, 14},
                {17, 14},
                {18, 14},
                {9,  15},
                {9,  16},
                {9,  17},
                {9,  18},
                {9,  19},
                {9,  20},
                {9,  21},
                {9,  22},
                {18, 15},
                {18, 16},
                {18, 17},
                {18, 18},
                {18, 19},
                {18, 20},
                {18, 21},
                {18, 22},
                {0,  17},
                {1,  17},
                {2,  17},
                {3,  17},
                {4,  17},
                {5,  17},
                {22, 17},
                {23, 17},
                {24, 17},
                {25, 17},
                {26, 17},
                {27, 17},
                {10, 20},
                {11, 20},
                {12, 20},
                {13, 20},
                {14, 20},
                {15, 20},
                {16, 20},
                {17, 20},
                {1,  23},
                {2,  23},
                {3,  23},
                {4,  23},
                {5,  23},
                {6,  23},
                {7,  23},
                {8,  23},
                {9,  23},
                {10, 23},
                {11, 23},
                {12, 23},
                {15, 23},
                {16, 23},
                {17, 23},
                {18, 23},
                {19, 23},
                {20, 23},
                {21, 23},
                {22, 23},
                {23, 23},
                {24, 23},
                {25, 23},
                {26, 23},
                {1,  24},
                {1,  25},
                {6,  24},
                {6,  25},
                {12, 24},
                {12, 25},
                {15, 24},
                {15, 25},
                {21, 24},
                {21, 25},
                {26, 24},
                {26, 25},
                {1,  26},
                {2,  26},
                {3,  26},
                {24, 26},
                {25, 26},
                {26, 26},
                {6,  26},
                {7,  26},
                {8,  26},
                {9,  26},
                {10, 26},
                {11, 26},
                {12, 26},
                {13, 26},
                {14, 26},
                {15, 26},
                {16, 26},
                {17, 26},
                {18, 26},
                {19, 26},
                {20, 26},
                {21, 26},
                {3,  27},
                {3,  28},
                {6,  27},
                {6,  28},
                {9,  27},
                {9,  28},
                {18, 27},
                {18, 28},
                {21, 27},
                {21, 28},
                {24, 27},
                {24, 28},
                {1,  29},
                {2,  29},
                {3,  29},
                {4,  29},
                {5,  29},
                {6,  29},
                {9,  29},
                {10, 29},
                {11, 29},
                {12, 29},
                {15, 29},
                {16, 29},
                {17, 29},
                {18, 29},
                {21, 29},
                {22, 29},
                {23, 29},
                {24, 29},
                {25, 29},
                {26, 29},
                {1,  30},
                {1,  31},
                {12, 30},
                {12, 31},
                {15, 30},
                {15, 31},
                {26, 30},
                {26, 31},
                {1,  32},
                {2,  32},
                {3,  32},
                {4,  32},
                {5,  32},
                {6,  32},
                {7,  32},
                {8,  32},
                {9,  32},
                {10, 32},
                {11, 32},
                {12, 32},
                {13, 32},
                {14, 32},
                {15, 32},
                {16, 32},
                {17, 32},
                {18, 32},
                {19, 32},
                {20, 32},
                {21, 32},
                {22, 32},
                {23, 32},
                {24, 32},
                {25, 32},
                {26, 32},
                {19, 17},
                {20, 17},
                {8,  17},
                {7,  17}
        };
        
        for (int i = 0; i < NUM_ACCESSIBLE_TILES; ++i)
        {
            Tile *this_tile = &tile_map[accessible_tiles[i].x][accessible_tiles[i].y];
            this_tile->type = STRAIGHT_PATH;
            this_tile->has_tictac = true;
        }
    }
    
    //specify the access info of each junction tile
    {
        JunctionTileInfo junction_tiles[NUM_JUNCTION_TILES] = {0};
        
        junction_tiles[0] = (JunctionTileInfo) {
                .x=1, .y=4,
                .up_access = false, .right_access = true,
                .down_access = true, .left_access = false
        };
        junction_tiles[1] = (JunctionTileInfo) {
                6, 4, false, true, true, true
        };
        junction_tiles[2] = (JunctionTileInfo) {
                12, 4, false, false, true, true
        };
        junction_tiles[3] = (JunctionTileInfo) {
                15, 4, 0, 1, 1, 0
        };
        junction_tiles[4] = (JunctionTileInfo) {
                21, 4, 0, 1, 1, 1
        };
        junction_tiles[5] = (JunctionTileInfo) {
                26, 4, 0, 0, 1, 1
        };
        junction_tiles[6] = (JunctionTileInfo) {
                1, 8, 1, 1, 1, 0
        };
        junction_tiles[7] = (JunctionTileInfo) {
                6, 8, 1, 1, 1, 1
        };
        junction_tiles[8] = (JunctionTileInfo) {
                9, 8, 0, 1, 1, 1
        };
        junction_tiles[9] = (JunctionTileInfo) {
                12, 8, 1, 1, 0, 1
        };
        junction_tiles[10] = (JunctionTileInfo) {
                15, 8, 1, 1, 0, 1
        };
        junction_tiles[11] = (JunctionTileInfo) {
                18, 8, 0, 1, 1, 1
        };
        junction_tiles[12] = (JunctionTileInfo) {
                21, 8, 1, 1, 1, 1
        };
        junction_tiles[13] = (JunctionTileInfo) {
                26, 8, 1, 0, 1, 1
        };
        junction_tiles[14] = (JunctionTileInfo) {
                1, 11, 1, 1, 0, 0
        };
        junction_tiles[15] = (JunctionTileInfo) {
                6, 11, 1, 0, 1, 1
        };
        junction_tiles[16] = (JunctionTileInfo) {
                9, 11, 1, 1, 0, 0
        };
        junction_tiles[17] = (JunctionTileInfo) {
                12, 11, 0, 0, 1, 1
        };
        junction_tiles[18] = (JunctionTileInfo) {
                15, 11, 0, 1, 1, 0
        };
        junction_tiles[19] = (JunctionTileInfo) {
                18, 11, 1, 0, 0, 1
        };
        junction_tiles[20] = (JunctionTileInfo) {
                21, 11, 1, 1, 1, 0
        };
        junction_tiles[21] = (JunctionTileInfo) {
                26, 11, 1, 0, 0, 1
        };
        junction_tiles[22] = (JunctionTileInfo) {
                9, 14, 0, 1, 1, 0
        };
        junction_tiles[23] = (JunctionTileInfo) {
                12, 14, 1, 1, 0, 1, 1
        };
        junction_tiles[24] = (JunctionTileInfo) {
                15, 14, 1, 1, 0, 1, 1
        };
        junction_tiles[25] = (JunctionTileInfo) {
                18, 14, 0, 0, 1, 1
        };
        junction_tiles[26] = (JunctionTileInfo) {
                6, 17, 1, 1, 1, 1
        };
        junction_tiles[27] = (JunctionTileInfo) {
                9, 17, 1, 0, 1, 1
        };
        junction_tiles[28] = (JunctionTileInfo) {
                18, 17, 1, 1, 1, 0
        };
        junction_tiles[29] = (JunctionTileInfo) {
                21, 17, 1, 1, 1, 1
        };
        junction_tiles[30] = (JunctionTileInfo) {
                9, 20, 1, 1, 1, 0
        };
        junction_tiles[31] = (JunctionTileInfo) {
                18, 20, 1, 0, 1, 1
        };
        junction_tiles[32] = (JunctionTileInfo) {
                1, 23, 0, 1, 1, 0
        };
        junction_tiles[33] = (JunctionTileInfo) {
                6, 23, 1, 1, 1, 1
        };
        junction_tiles[34] = (JunctionTileInfo) {
                9, 23, 1, 1, 0, 1
        };
        junction_tiles[35] = (JunctionTileInfo) {
                12, 23, 0, 0, 1, 1
        };
        junction_tiles[36] = (JunctionTileInfo) {
                15, 23, 0, 1, 1, 0
        };
        junction_tiles[37] = (JunctionTileInfo) {
                21, 23, 1, 1, 1, 1
        };
        junction_tiles[38] = (JunctionTileInfo) {
                26, 23, 0, 0, 1, 1
        };
        junction_tiles[39] = (JunctionTileInfo) {
                1, 26, 1, 1, 0, 0
        };
        junction_tiles[40] = (JunctionTileInfo) {
                3, 26, 0, 0, 1, 1
        };
        junction_tiles[41] = (JunctionTileInfo) {
                6, 26, 1, 1, 1, 0
        };
        junction_tiles[42] = (JunctionTileInfo) {
                9, 26, 0, 1, 1, 1
        };
        junction_tiles[43] = (JunctionTileInfo) {
                12, 26, 1, 1, 0, 1, 1
        };
        junction_tiles[44] = (JunctionTileInfo) {
                15, 26, 1, 1, 0, 1, 1
        };
        junction_tiles[45] = (JunctionTileInfo) {
                18, 26, 0, 1, 1, 1
        };
        junction_tiles[46] = (JunctionTileInfo) {
                21, 26, 1, 0, 1, 1
        };
        junction_tiles[47] = (JunctionTileInfo) {
                24, 26, 0, 1, 1, 0
        };
        junction_tiles[48] = (JunctionTileInfo) {
                26, 26, 1, 0, 0, 1
        };
        junction_tiles[49] = (JunctionTileInfo) {
                1, 29, 0, 1, 1, 0
        };
        junction_tiles[50] = (JunctionTileInfo) {
                3, 29, 1, 1, 0, 1
        };
        junction_tiles[51] = (JunctionTileInfo) {
                6, 29, 1, 0, 0, 1
        };
        junction_tiles[52] = (JunctionTileInfo) {
                9, 29, 1, 1, 0, 0
        };
        junction_tiles[53] = (JunctionTileInfo) {
                12, 29, 0, 0, 1, 1
        };
        junction_tiles[54] = (JunctionTileInfo) {
                15, 29, 0, 1, 1, 0
        };
        junction_tiles[55] = (JunctionTileInfo) {
                18, 29, 1, 0, 0, 1
        };
        junction_tiles[56] = (JunctionTileInfo) {
                21, 29, 1, 1, 0, 0
        };
        junction_tiles[57] = (JunctionTileInfo) {
                24, 29, 1, 1, 0, 1
        };
        junction_tiles[58] = (JunctionTileInfo) {
                26, 29, 0, 0, 1, 1
        };
        junction_tiles[59] = (JunctionTileInfo) {
                1, 32, 1, 1, 0, 0
        };
        junction_tiles[60] = (JunctionTileInfo) {
                12, 32, 1, 1, 0, 1
        };
        junction_tiles[61] = (JunctionTileInfo) {
                15, 32, 1, 1, 0, 1
        };
        junction_tiles[62] = (JunctionTileInfo) {
                26, 32, 1, 0, 0, 1
        };
        junction_tiles[63] = (JunctionTileInfo) {
                18, 23, 1, 1, 0, 1
        };
        for (int i = 0; i < NUM_JUNCTION_TILES; ++i)
        {
            Tile *this_tile = &tile_map[junction_tiles[i].x][junction_tiles[i].y];
            
            this_tile->type = JUNCTION;
            
            this_tile->x = junction_tiles[i].x;
            this_tile->y = junction_tiles[i].y;
            
            this_tile->junction_info.left_access = junction_tiles[i].left_access;
            this_tile->junction_info.up_access = junction_tiles[i].up_access;
            this_tile->junction_info.right_access = junction_tiles[i].right_access;
            this_tile->junction_info.down_access = junction_tiles[i].down_access;
            this_tile->junction_info.special_access = junction_tiles[i].special_access;
        }
    }
    
    //TODO: unset tic_tacs where they shouldn't be set
    
    //store coordinates of each tile
    for (int i = 0; i < TILES_IN_ROW; ++i)
    {
        for (int j = 0; j < TILES_IN_COLUMN; ++j)
        {
            tile_map[i][j].x = i;
            tile_map[i][j].y = j;
            if (i == 1 && j == 6 || i == 26 && j == 6 || i == 1 && j == 26 || i == 26 && j == 26){
                tile_map[i][j].has_energiser = true;
            }
        }
    }
}

void reset_level()
{
    reset_tiles();
    player_won = false;
    score = 0;
    pacman_lives = 3;
    blinky_r.mode_times_index = 0;
    blinky_r.timer = mode_times_lv1[blinky_r.mode_times_index++];
    blinky_r.last_get_time = 0;
}

void reset_actor_positions(){
    pacman_centre = (Vector2) {SCREEN_WIDTH / 2, TILE_LENGTH * 26 + TILE_LENGTH / 2};
    pacman_direction = LEFT;
    
    blinky_r.mode = SCATTER;
    blinky_r.centre_pos = (Vector2) {SCREEN_WIDTH/2, TILE_LENGTH * 14 + TILE_LENGTH / 2};
    
    blinky_r.scatter_target = (Vector2Int){25,0};
    blinky_r.direction = LEFT;
}

void kill_player(){
    pacman_lives -= 1;
    if (pacman_lives != 0){reset_actor_positions();}
}

float distance_between_tiles(Vector2Int a, Vector2Int b){
    Vector2 tile_centre_a = {a.x * TILE_LENGTH + TILE_LENGTH / 2,
                             a.y * TILE_LENGTH + TILE_LENGTH / 2};
    Vector2 tile_centre_b = {b.x * TILE_LENGTH + TILE_LENGTH / 2,
                             b.y * TILE_LENGTH + TILE_LENGTH / 2};
    
    return Vector2Distance(tile_centre_a, tile_centre_b);
}


int main(void)
{
    // Initialization
    int tictac_total = NUM_ACCESSIBLE_TILES;
    bool energized = false;
    bool begin_energized = false;
    float energize_time = 7;
    float energize_start_time;
    float time_since_energized;
    
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "EAT EM UP!");
    
    reset_level();
    reset_actor_positions();
    
    
    
    SetTargetFPS(100); // Set our game to run at 60 frames-per-second
    // Main game loop
    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        //update variables here
        Tile* pacman_tile = &tile_map[(int) pacman_centre.x / TILE_LENGTH]
        [(int) pacman_centre.y / TILE_LENGTH];
    
        if (!player_won && pacman_lives != 0){
            if(begin_energized){
                energized = true;
                begin_energized = false;
                if (blinky_r.mode != EATEN){
                    blinky_r.mode_before_eaten = blinky_r.mode;
                    blinky_r.mode = FRIGHTENED;
                }
                energize_start_time = GetTime();
                time_since_energized = 0;
            }
            else if (energized){
                time_since_energized = GetTime() - energize_start_time;
                if (time_since_energized >= energize_time){ energized = false;
                blinky_r.mode = blinky_r.mode_before_eaten;}
            }
            
            
            //move pacman
            {
                if (pacman_tile->has_tictac)
                {
                    pacman_tile->has_tictac = false;
                    score += TAC_POINTS;
                    if (pacman_tile->has_energiser){
                        begin_energized = true;
                        pacman_tile->has_energiser = false;
                    }
                }
            
            
                if (pacman_tile->type == STRAIGHT_PATH)
                {
                    switch (pacman_direction)
                    {
                        case UP:
                            if (IsKeyPressed(KEY_DOWN)) { pacman_direction = DOWN; }
                            break;
                        case DOWN:
                            if (IsKeyPressed(KEY_UP)) { pacman_direction = UP; }
                            break;
                        case RIGHT:
                            if (IsKeyPressed(KEY_LEFT)) { pacman_direction = LEFT; }
                            break;
                        case LEFT:
                            if (IsKeyPressed((KEY_RIGHT))) { pacman_direction = RIGHT; }
                            break;
                    }
                
                    //wrap pac around if he goes out of bounds
                    if (pacman_centre.x >= SCREEN_WIDTH - 1) { pacman_centre.x = 1; }
                    if (pacman_centre.x < 0) { pacman_centre.x = SCREEN_WIDTH - 1; }
                }
                else if (pacman_tile->type == JUNCTION)
                {
                    Vector2 tile_centre = {
                            pacman_tile->x * TILE_LENGTH + (TILE_LENGTH / 2),
                            pacman_tile->y * TILE_LENGTH + (TILE_LENGTH / 2)
                    };
                
                    if (IsKeyDown(KEY_DOWN) || pacman_direction == DOWN)
                    {
                        if (pacman_tile->junction_info.down_access ||
                            pacman_centre.y < tile_centre.y) { pacman_direction = DOWN; }
                        else if (pacman_direction == DOWN)
                        {
                            pacman_direction = NONE;
                            if (pacman_centre.y >
                                tile_centre.y) { pacman_centre.y -= PACMAN_SPEED; }
                        }
                    }
                    if (IsKeyDown((KEY_UP)) || pacman_direction == UP)
                    {
                        if (pacman_tile->junction_info.up_access ||
                            pacman_centre.y > tile_centre.y)
                        {
                            pacman_direction = UP;
                        }
                        else if (pacman_direction == UP)
                        {
                            pacman_direction = NONE;
                            if (pacman_centre.y <
                                tile_centre.y) { pacman_centre.y += PACMAN_SPEED; }
                        }
                    }
                    if (IsKeyDown(KEY_RIGHT) || pacman_direction == RIGHT)
                    {
                        if (pacman_tile->junction_info.right_access ||
                            pacman_centre.x < tile_centre.x) { pacman_direction = RIGHT; }
                        else if (pacman_direction == RIGHT)
                        {
                            pacman_direction = NONE;
                            if (pacman_centre.x <
                                tile_centre.x) { pacman_centre.x += PACMAN_SPEED; }
                        }
                    }
                    if (IsKeyDown((KEY_LEFT)) || pacman_direction == LEFT)
                    {
                        if (pacman_tile->junction_info.left_access ||
                            pacman_centre.x > tile_centre.x)
                        {
                            pacman_direction = LEFT;
                        }
                        else if (pacman_direction == LEFT)
                        {
                            pacman_direction = NONE;
                            if (pacman_centre.x >
                                tile_centre.x) { pacman_centre.x -= PACMAN_SPEED; }
                        }
                    }
                
                    //bring pac to centre as he turns
                    switch (pacman_direction)
                    {
                        case UP:
                        case DOWN:
                            if (pacman_centre.x != tile_centre.x)
                            {
                                pacman_centre.x += (pacman_centre.x < tile_centre.x) ? 1
                                                                                     : -1;
                            }
                            break;
                        case RIGHT:
                        case LEFT:
                            if (pacman_centre.y != tile_centre.y)
                            {
                                pacman_centre.y += (pacman_centre.y < tile_centre.y) ? 1
                                                                                     : -1;
                            }
                            break;
                    
                    }
                }
            
                if (pacman_tile->type != INACCESSIBLE)
                {
                    switch (pacman_direction)
                    {
                        case LEFT:
                            pacman_centre.x -= PACMAN_SPEED;
                            break;
                        case RIGHT:
                            pacman_centre.x += PACMAN_SPEED;
                            break;
                        case UP:
                            pacman_centre.y -= PACMAN_SPEED;
                            break;
                        case DOWN:
                            pacman_centre.y += PACMAN_SPEED;
                            break;
                        case NONE:
                            break;
                    }
                
                }
            }
        
            //move blinky
            {
                // time to change mode
                if (blinky_r.timer <= 0){
                    blinky_r.timer = mode_times_lv1[blinky_r.mode_times_index];
                    blinky_r.mode_times_index++;
                    blinky_r.last_get_time = GetTime();
    
                    if (blinky_r.mode == SCATTER) { blinky_r.mode = CHASE; }
                    else if (blinky_r.mode == CHASE) { blinky_r.mode = SCATTER; }
                    blinky_r.mode_switch = true;
                }
                
                //timer only counts down while not ded
                if (blinky_r.mode != EATEN && blinky_r.mode != FRIGHTENED){
                    blinky_r.timer -= GetTime() - blinky_r.last_get_time;
                    blinky_r.last_get_time = GetTime();
                }
                else{
                    if (blinky_r.current_tile.x == BLINKY_START_TILE.x &&
                        blinky_r.current_tile.y == BLINKY_START_TILE.y){
                        blinky_r.mode = blinky_r.mode_before_eaten;
                        blinky_r.direction = LEFT;
                    }
                    
                }
                
                blinky_r.current_tile = tile_map[(int) blinky_r.centre_pos.x /TILE_LENGTH]
                                                [(int) blinky_r.centre_pos.y / TILE_LENGTH];
                
                Vector2 tile_centre = {
                        blinky_r.current_tile.x * TILE_LENGTH + (TILE_LENGTH / 2),
                        blinky_r.current_tile.y * TILE_LENGTH + (TILE_LENGTH / 2)};
            
                if (blinky_r.current_tile.type == JUNCTION)
                {
                    //copy tile access info into an enum addressable array
                    blinky_r.move_choices[UP] = blinky_r.current_tile.junction_info.special_access
                                                ? false
                                                : blinky_r.current_tile.junction_info.up_access;
                    blinky_r.move_choices[RIGHT] = blinky_r.current_tile.junction_info.right_access;
                    blinky_r.move_choices[DOWN] = blinky_r.current_tile.junction_info.down_access;
                    blinky_r.move_choices[LEFT] = blinky_r.current_tile.junction_info.left_access;
                
                    //find out if at the centre of the junction and don't let them reverse
                    //TODO: rewrite so that blinky's speed doesn't have to be 1,
                    bool at_centre = false;
                    {
                        if (blinky_r.centre_pos.x == tile_centre.x)
                        {
                            if (blinky_r.direction == LEFT)
                            {
                                at_centre = true;
                                blinky_r.move_choices[RIGHT] = false;
                            }
                            else if (blinky_r.direction == RIGHT)
                            {
                                at_centre = true;
                                blinky_r.move_choices[LEFT] = false;
                            }
                        }
                        if (blinky_r.centre_pos.y == tile_centre.y)
                        {
                            if (blinky_r.direction == UP)
                            {
                                at_centre = true;
                                blinky_r.move_choices[DOWN] = false;
                            }
                            else if (blinky_r.direction == DOWN)
                            {
                                at_centre = true;
                                blinky_r.move_choices[UP] = false;
                            }
                        }
                    }
                
                    if (at_centre)
                    {
                        if (blinky_r.mode == CHASE)
                        {
                            blinky_r.current_target = (Vector2Int) {pacman_tile->x,
                                                                    pacman_tile->y};
                        }
                        else if (blinky_r.mode == SCATTER)
                        {
                            blinky_r.current_target = blinky_r.scatter_target;
                        }
                    
                        //check each available direction for the tile that is closest to the target tile
                        enum Direction decision;
                        if (blinky_r.mode != FRIGHTENED){
                            float nearest_distance = INFINITY;
                            float temp_distance;
                        
                            if (blinky_r.move_choices[UP])
                            {
                                Vector2Int tile_above = {
                                        blinky_r.current_tile.x,
                                        blinky_r.current_tile.y - 1
                                };
                                temp_distance = distance_between_tiles(
                                        blinky_r.current_target,
                                        tile_above);
                            
                                if (temp_distance < nearest_distance)
                                {
                                    nearest_distance = temp_distance;
                                    decision = UP;
                                }
                            }
                            if (blinky_r.move_choices[RIGHT])
                            {
                                Vector2Int tile_to_right = {
                                        blinky_r.current_tile.x + 1,
                                        blinky_r.current_tile.y
                                };
                                temp_distance = distance_between_tiles(
                                        blinky_r.current_target,
                                        tile_to_right);
                            
                                if (temp_distance < nearest_distance)
                                {
                                    nearest_distance = temp_distance;
                                    decision = RIGHT;
                                }
                            }
                            if (blinky_r.move_choices[DOWN])
                            {
                                Vector2Int tile_below = {
                                        blinky_r.current_tile.x,
                                        blinky_r.current_tile.y + 1
                                };
                                temp_distance = distance_between_tiles(
                                        blinky_r.current_target,
                                        tile_below);
                            
                                if (temp_distance < nearest_distance)
                                {
                                    nearest_distance = temp_distance;
                                    decision = DOWN;
                                }
                            }
                            if (blinky_r.move_choices[LEFT])
                            {
                                Vector2Int tile_to_left = {
                                        blinky_r.current_tile.x - 1,
                                        blinky_r.current_tile.y
                                };
                                temp_distance = distance_between_tiles(
                                        blinky_r.current_target,
                                        tile_to_left);
                            
                                if (temp_distance < nearest_distance)
                                {
                                    nearest_distance = temp_distance;
                                    decision = LEFT;
                                }
                            }
                        }
                        else{
                            int num = GetRandomValue(0, 3);
                            while (!blinky_r.move_choices[num]) {num = GetRandomValue(0, 3);}
                            decision = num;
                        }
                    
                        blinky_r.direction = decision;
                    }
                }
            
            
                if (blinky_r.current_tile.type != INACCESSIBLE)
                {
                    if (blinky_r.mode_switch)
                    {
                        switch (blinky_r.direction)
                        {
                            case UP:
                                blinky_r.direction = DOWN;
                                break;
                            case RIGHT:
                                blinky_r.direction = LEFT;
                                break;
                            case DOWN:
                                blinky_r.direction = UP;
                                break;
                            case LEFT:
                                blinky_r.direction = RIGHT;
                                break;
                        }
                    
                        blinky_r.mode_switch = false;
                    }
                
                    switch (blinky_r.direction)
                    {
                        case LEFT:
                            blinky_r.centre_pos.x -= BLINKY_SPEED;
                            break;
                        case RIGHT:
                            blinky_r.centre_pos.x += BLINKY_SPEED;
                            break;
                        case UP:
                            blinky_r.centre_pos.y -= BLINKY_SPEED;
                            break;
                        case DOWN:
                            blinky_r.centre_pos.y += BLINKY_SPEED;
                            break;
                        case NONE:
                            break;
                    }
                }
    
                if (blinky_r.current_tile.x == pacman_tile->x &&
                    blinky_r.current_tile.y == pacman_tile->y)
                {
                    if (blinky_r.mode == FRIGHTENED)
                    {
                        blinky_r.mode = EATEN;
                        blinky_r.current_target = BLINKY_START_TILE;
                    }
                    else if (blinky_r.mode == EATEN){}
                    else { kill_player(); }
                    
                    
                }
    
                if (blinky_r.centre_pos.x >= SCREEN_WIDTH - 1) { blinky_r.centre_pos.x = 1; }
                if (blinky_r.centre_pos.x < 0) { blinky_r.centre_pos.x = SCREEN_WIDTH - 1; }
            }
        }
        else if (IsKeyPressed(KEY_SPACE)){
            reset_level();
            reset_actor_positions();
        }
        
        if (score == NUM_ACCESSIBLE_TILES * TAC_POINTS){
            player_won = true;
        }
        
        BeginDrawing();
        
        ClearBackground(BLACK);
        
        
        for (int i = 0; i < TILES_IN_ROW; ++i)
        {
            for (int j = 0; j < TILES_IN_COLUMN; ++j)
            {
                Color tile_indicator = BLACK;
                #ifndef NDEBUG
                tile_indicator = DARKGREEN;
                #endif
                if (tile_map[i][j].type == STRAIGHT_PATH){tile_indicator = GOLD;}
                else if (tile_map[i][j].type == JUNCTION){
                    #ifndef NDEBUG
                    tile_indicator = RED;
                    #else
                    tile_indicator = GOLD;
                    #endif
                }
                
                DrawRectangleLines(i * TILE_LENGTH,
                                   j * TILE_LENGTH,
                                   TILE_LENGTH, TILE_LENGTH, tile_indicator);
                
                if (tile_map[i][j].has_tictac)
                {
                    DrawRectangle(i * TILE_LENGTH + (TILE_LENGTH / 2) - 1,
                                  j * TILE_LENGTH + (TILE_LENGTH / 2) - 1, 3, 3, BEIGE);
                    if (tile_map[i][j].has_energiser){
                        DrawCircle(i * TILE_LENGTH + (TILE_LENGTH / 2) ,
                                   j * TILE_LENGTH + (TILE_LENGTH / 2) , TILE_LENGTH/2,
                                   BEIGE);
                    }
                }
            }
            
        }
    
        #ifndef NDEBUG
        for (int i = 0; i < TILES_IN_ROW; ++i)
        {
            DrawText(FormatText("%i", i), i * TILE_LENGTH, 0, 5, RAYWHITE);
        }
        for (int i = 0; i < TILES_IN_COLUMN; ++i)
        {
            DrawText(FormatText("%i", i), 0, i * TILE_LENGTH, 5, RAYWHITE);
        }
    
        DrawText(FormatText("blinky timer: %.2f", blinky_r.timer), SCREEN_WIDTH - 300, 45,
                 20, BLUE);
        DrawText(FormatText((blinky_r.mode ? "mode: CHASE" : "mode: SCATTER")), SCREEN_WIDTH - 300, 60,
                 20, BLUE);
        DrawText((FormatText("framerate: %i", GetFPS())), SCREEN_WIDTH - 300,
                 SCREEN_HEIGHT - 45, 20, BLUE);
        #endif
        
        DrawText(FormatText("score: %i", score), 70, 45, 30, RAYWHITE);
        
        //draw actors
        DrawCircleV(pacman_centre, ((float)TILE_LENGTH / 8) * 5, YELLOW);
        Color blinky_color = RED;
        if (blinky_r.mode == FRIGHTENED) { blinky_color = BLUE; }
        else if (blinky_r.mode == EATEN) { blinky_color = BLACK; }
        DrawCircleV(blinky_r.centre_pos, ((float) TILE_LENGTH / 8) * 5,
                    blinky_color);
        
        //victory and game over text
        if (player_won || pacman_lives == 0)
        {
            DrawText(player_won ? "YOU WON!" : "GAME OVER", SCREEN_WIDTH / 2 - 60, 35, 40,
                     player_won ? YELLOW : RED);
            DrawText("press SPACE to restart", SCREEN_WIDTH - 400, SCREEN_HEIGHT - 45, 20,
                     player_won ? YELLOW : RED);
        }
        
        DrawText(FormatText("lives: %i", pacman_lives), 70, SCREEN_HEIGHT - 50, 30, RAYWHITE);
        
        EndDrawing();
    }
    
    // De-Initialization
    
    CloseWindow(); // Close window and OpenGL context
    
    return 0;
}