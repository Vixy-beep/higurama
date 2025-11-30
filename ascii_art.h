#ifndef ASCII_ART_H
#define ASCII_ART_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// ANSI Colors
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_MAGENTA "\033[1;35m"
#define COLOR_CYAN    "\033[1;36m"
#define COLOR_WHITE   "\033[1;37m"

// Clear screen and move cursor to top
#define CLEAR_SCREEN() printf("\033[2J\033[H")

// ====================================================================================
// SPLASH SCREEN FRAMES - Anime style spinning animation
// ====================================================================================

// Lucky Star opening inspired - Character spinning animation (6 frames)
const char *splash_frames[] = {
    // Frame 1 - Front facing
    COLOR_CYAN
    "                    ⠀⠀⠀⢀⣀⣀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿⣿" COLOR_YELLOW "⣀⣀⠀⠀⣀⣀" COLOR_CYAN "⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿" COLOR_YELLOW "⢸⣿⡇⠀⢸⣿⡇" COLOR_CYAN "⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⠸⣿⣿⣿" COLOR_YELLOW "⠉⠁⠀⠀⠈⠉" COLOR_CYAN "⣿⣿⣿⠇⠀⠀⠀⠀⠀\n"
    "                    ⠀⢻⣿⣿⣿" COLOR_RED "⣿⣿⣿⣿" COLOR_CYAN "⣿⣿⣿⡟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠀⠈⠛⠿⠿⠿⠿⠛⠁⠀⠀⠀⠀⠀⠀\n"
    COLOR_MAGENTA "                      ♪ HIGURASHI ♪\n" COLOR_RESET,

    // Frame 2 - Slight turn left
    COLOR_CYAN
    "                    ⠀⠀⠀⢀⣀⣀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿" COLOR_YELLOW "⣀⣀⠀⠀⠀⣀" COLOR_CYAN "⣿⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿" COLOR_YELLOW "⢸⣿⡇⠀⠀⢸⣿" COLOR_CYAN "⣿⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⠸⣿⣿" COLOR_YELLOW "⠉⠁⠀⠀⠀⠈⠉" COLOR_CYAN "⣿⣿⠇⠀⠀⠀⠀⠀\n"
    "                    ⠀⢻⣿⣿" COLOR_RED "⣿⣿⣿⣿⣿" COLOR_CYAN "⣿⣿⡟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠀⠈⠛⠿⠿⠿⠿⠛⠁⠀⠀⠀⠀⠀⠀\n"
    COLOR_YELLOW "                      ♫ LOADING ♫\n" COLOR_RESET,

    // Frame 3 - Side view left
    COLOR_CYAN
    "                    ⠀⠀⠀⢀⣀⣀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿" COLOR_YELLOW "⣀⠀⠀⠀⠀⠀⠀" COLOR_CYAN "⣿⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⢸" COLOR_YELLOW "⢸⣿⡇⠀⠀⠀⠀⠀" COLOR_CYAN "⣿⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⠸" COLOR_YELLOW "⠈⠉⠁⠀⠀⠀⠀⠀⠀" COLOR_CYAN "⣿⠇⠀⠀⠀⠀⠀\n"
    "                    ⠀⢻" COLOR_RED "⣿⣿⣿⣿⣿⣿" COLOR_CYAN "⣿⣿⡟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠀⠈⠛⠿⠿⠿⠿⠛⠁⠀⠀⠀⠀⠀⠀\n"
    COLOR_GREEN "                      ★ SYSTEM ★\n" COLOR_RESET,

    // Frame 4 - Back view
    COLOR_CYAN
    "                    ⠀⠀⠀⢀⣀⣀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⠸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠇⠀⠀⠀⠀⠀\n"
    "                    ⠀⢻⣿⣿⣿" COLOR_MAGENTA "▓▓▓" COLOR_CYAN "⣿⣿⣿⣿⡟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠀⠈⠛⠿⠿⠿⠿⠛⠁⠀⠀⠀⠀⠀⠀\n"
    COLOR_RED "                      ▸ READY ◂\n" COLOR_RESET,

    // Frame 5 - Side view right
    COLOR_CYAN
    "                    ⠀⠀⠀⢀⣀⣀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿⣿" COLOR_YELLOW "⠀⠀⠀⠀⠀⠀⣀" COLOR_CYAN "⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿⣿" COLOR_YELLOW "⠀⠀⠀⠀⢸⣿⡇" COLOR_CYAN "⡇⠀⠀⠀⠀⠀\n"
    "                    ⠸⣿⠇" COLOR_YELLOW "⠀⠀⠀⠀⠀⠀⠈⠉⠁" COLOR_CYAN "⠇⠀⠀⠀⠀⠀\n"
    "                    ⠀⢻⣿⣿" COLOR_RED "⣿⣿⣿⣿⣿⣿" COLOR_CYAN "⡟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠀⠈⠛⠿⠿⠿⠿⠛⠁⠀⠀⠀⠀⠀⠀\n"
    COLOR_BLUE "                      ◉ START ◉\n" COLOR_RESET,

    // Frame 6 - Slight turn right (completing rotation)
    COLOR_CYAN
    "                    ⠀⠀⠀⢀⣀⣀⣀⣀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⢠⣾⣿⣿⣿⣿⣿⣿⣿⣷⡄⠀⠀⠀⠀⠀⠀\n"
    "                    ⠀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿⣿" COLOR_YELLOW "⣀⠀⠀⠀⣀⣀" COLOR_CYAN "⣿⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⢸⣿⣿⣿" COLOR_YELLOW "⣿⡇⠀⢸⣿⡇" COLOR_CYAN "⣿⡇⠀⠀⠀⠀⠀\n"
    "                    ⠸⣿⠇" COLOR_YELLOW "⠈⠉⠀⠀⠀⠈⠉" COLOR_CYAN "⣿⣿⠇⠀⠀⠀⠀⠀\n"
    "                    ⠀⢻⣿⣿" COLOR_RED "⣿⣿⣿⣿⣿" COLOR_CYAN "⣿⣿⡟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠻⣿⣿⣿⣿⣿⣿⣿⣿⠟⠀⠀⠀⠀⠀\n"
    "                    ⠀⠀⠀⠈⠛⠿⠿⠿⠿⠛⠁⠀⠀⠀⠀⠀⠀\n"
    COLOR_MAGENTA "                      ♬ GO! ♬\n" COLOR_RESET,
    
    NULL
};

// Title banner for Higurashi bot
const char *higurashi_banner = 
    COLOR_RED
    "    ██╗  ██╗██╗ ██████╗ ██╗   ██╗██████╗  █████╗ ███████╗██╗  ██╗██╗\n"
    "    ██║  ██║██║██╔════╝ ██║   ██║██╔══██╗██╔══██╗██╔════╝██║  ██║██║\n"
    "    ███████║██║██║  ███╗██║   ██║██████╔╝███████║███████╗███████║██║\n"
    "    ██╔══██║██║██║   ██║██║   ██║██╔══██╗██╔══██║╚════██║██╔══██║██║\n"
    "    ██║  ██║██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║███████║██║  ██║██║\n"
    "    ╚═╝  ╚═╝╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚═╝\n"
    COLOR_CYAN
    "              ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
    "                    🌸 IoT Hunter • Mirai Evolution 2.0 🌸\n"
    "              ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
    COLOR_RESET;

// Higurama C2 banner
const char *higurama_banner =
    COLOR_MAGENTA
    "    ██╗  ██╗██╗ ██████╗ ██╗   ██╗██████╗  █████╗ ███╗   ███╗ █████╗ \n"
    "    ██║  ██║██║██╔════╝ ██║   ██║██╔══██╗██╔══██╗████╗ ████║██╔══██╗\n"
    "    ███████║██║██║  ███╗██║   ██║██████╔╝███████║██╔████╔██║███████║\n"
    "    ██╔══██║██║██║   ██║██║   ██║██╔══██╗██╔══██║██║╚██╔╝██║██╔══██║\n"
    "    ██║  ██║██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║\n"
    "    ╚═╝  ╚═╝╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝\n"
    COLOR_YELLOW
    "         ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
    "              ⚡ Master Command & Control ⚡ Mirai 2.0+++++ ⚡\n"
    "         ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
    COLOR_RESET;

// ====================================================================================
// SPINNER FRAMES - Anime-style loading animation
// ====================================================================================

const char *spinner_frames[] = {
    COLOR_CYAN "⠋" COLOR_RESET,
    COLOR_CYAN "⠙" COLOR_RESET,
    COLOR_CYAN "⠹" COLOR_RESET,
    COLOR_CYAN "⠸" COLOR_RESET,
    COLOR_CYAN "⠼" COLOR_RESET,
    COLOR_CYAN "⠴" COLOR_RESET,
    COLOR_CYAN "⠦" COLOR_RESET,
    COLOR_CYAN "⠧" COLOR_RESET,
    COLOR_CYAN "⠇" COLOR_RESET,
    COLOR_CYAN "⠏" COLOR_RESET,
    NULL
};

// Kawaii faces for different states
const char *kawaii_scanning = COLOR_CYAN "(◕‿◕)" COLOR_RESET " Scanning network...";
const char *kawaii_exploiting = COLOR_YELLOW "(｀・ω・´)" COLOR_RESET " Exploiting target...";
const char *kawaii_success = COLOR_GREEN "(ﾉ◕ヮ◕)ﾉ*:･ﾟ✧" COLOR_RESET " Success!";
const char *kawaii_failed = COLOR_RED "(╥﹏╥)" COLOR_RESET " Failed...";
const char *kawaii_thinking = COLOR_MAGENTA "(¬‿¬)" COLOR_RESET " Analyzing...";

// ====================================================================================
// REPLICATION ANIMATION - When bot replicates (inspired by particle explosion)
// ====================================================================================

const char *replication_frames[] = {
    // Frame 1 - Center core
    COLOR_YELLOW
    "\n"
    "                        ⠀⠀⠀⠀⠀⢀⣀⡀⠀⠀⠀⠀⠀⠀\n"
    "                        ⠀⠀⠀⠀⣴⣿⣿⣿⣦⠀⠀⠀⠀\n"
    "                        ⠀⠀⠀⢸⣿⣿" COLOR_RED "◉" COLOR_YELLOW "⣿⣿⡇⠀⠀⠀\n"
    "                        ⠀⠀⠀⠀⠻⣿⣿⣿⠟⠀⠀⠀⠀\n"
    "                        ⠀⠀⠀⠀⠀⠈⠉⠁⠀⠀⠀⠀⠀\n"
    COLOR_RESET,

    // Frame 2 - Small explosion
    COLOR_YELLOW
    "\n"
    "                        ⠀⠀⠀⠀" COLOR_RED "★" COLOR_YELLOW "⢀⣀⡀" COLOR_RED "★" COLOR_YELLOW "⠀⠀⠀\n"
    "                        ⠀⠀" COLOR_RED "✦" COLOR_YELLOW "⠀⣴⣿⣿⣿⣦⠀" COLOR_RED "✦" COLOR_YELLOW "⠀\n"
    "                        ⠀⠀⠀⢸⣿⣿" COLOR_RED "◉" COLOR_YELLOW "⣿⣿⡇⠀⠀⠀\n"
    "                        ⠀⠀" COLOR_RED "✦" COLOR_YELLOW "⠀⠻⣿⣿⣿⠟⠀" COLOR_RED "✦" COLOR_YELLOW "⠀\n"
    "                        ⠀⠀⠀⠀" COLOR_RED "★" COLOR_YELLOW "⠈⠉⠁" COLOR_RED "★" COLOR_YELLOW "⠀⠀⠀\n"
    COLOR_RESET,

    // Frame 3 - Medium explosion
    COLOR_YELLOW
    "\n"
    "                        ⠀" COLOR_RED "✦" COLOR_YELLOW "⠀" COLOR_GREEN "★" COLOR_YELLOW "⢀⣀⡀" COLOR_GREEN "★" COLOR_YELLOW "⠀" COLOR_RED "✦" COLOR_YELLOW "⠀\n"
    "                        " COLOR_GREEN "★" COLOR_YELLOW "⠀⠀⣴⣿⣿⣿⣦⠀⠀" COLOR_GREEN "★" COLOR_YELLOW "\n"
    "                        " COLOR_RED "✧" COLOR_YELLOW "⠀⢸⣿⣿" COLOR_RED "◉" COLOR_YELLOW "⣿⣿⡇⠀" COLOR_RED "✧" COLOR_YELLOW "\n"
    "                        " COLOR_GREEN "★" COLOR_YELLOW "⠀⠀⠻⣿⣿⣿⠟⠀⠀" COLOR_GREEN "★" COLOR_YELLOW "\n"
    "                        ⠀" COLOR_RED "✦" COLOR_YELLOW "⠀" COLOR_GREEN "★" COLOR_YELLOW "⠈⠉⠁" COLOR_GREEN "★" COLOR_YELLOW "⠀" COLOR_RED "✦" COLOR_YELLOW "⠀\n"
    COLOR_RESET,

    // Frame 4 - Large explosion
    COLOR_YELLOW
    "\n"
    "                        " COLOR_RED "★" COLOR_YELLOW "⠀" COLOR_GREEN "✦" COLOR_CYAN "★" COLOR_YELLOW "⣀⡀" COLOR_CYAN "★" COLOR_GREEN "✦" COLOR_YELLOW "⠀" COLOR_RED "★" COLOR_YELLOW "\n"
    "                        " COLOR_GREEN "✧" COLOR_CYAN "★" COLOR_YELLOW "⣴⣿⣿⣿⣦" COLOR_CYAN "★" COLOR_GREEN "✧" COLOR_YELLOW "\n"
    "                        " COLOR_CYAN "★" COLOR_YELLOW "⢸⣿⣿" COLOR_RED "◉" COLOR_YELLOW "⣿⣿⡇" COLOR_CYAN "★" COLOR_YELLOW "\n"
    "                        " COLOR_GREEN "✧" COLOR_CYAN "★" COLOR_YELLOW "⠻⣿⣿⣿⠟" COLOR_CYAN "★" COLOR_GREEN "✧" COLOR_YELLOW "\n"
    "                        " COLOR_RED "★" COLOR_YELLOW "⠀" COLOR_GREEN "✦" COLOR_CYAN "★" COLOR_YELLOW "⠉⠁" COLOR_CYAN "★" COLOR_GREEN "✦" COLOR_YELLOW "⠀" COLOR_RED "★" COLOR_YELLOW "\n"
    COLOR_RESET,

    // Frame 5 - Particles spreading
    "\n"
    COLOR_RED "★" COLOR_YELLOW "⠀⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀" COLOR_CYAN "✧" COLOR_YELLOW "⠀⠀⠀" COLOR_CYAN "✧" COLOR_YELLOW "⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀⠀" COLOR_RED "★" COLOR_YELLOW "\n"
    "⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀⠀⠀⠀⠀⠀⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀\n"
    COLOR_CYAN "✦" COLOR_YELLOW "⠀⠀⠀⢸⣿" COLOR_RED "◉" COLOR_YELLOW "⣿⡇⠀⠀⠀" COLOR_CYAN "✦" COLOR_YELLOW "\n"
    "⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀⠀⠀⠀⠀⠀⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀\n"
    COLOR_RED "★" COLOR_YELLOW "⠀⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀" COLOR_CYAN "✧" COLOR_YELLOW "⠀⠀⠀" COLOR_CYAN "✧" COLOR_YELLOW "⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀⠀" COLOR_RED "★" COLOR_YELLOW "\n"
    COLOR_RESET,

    // Frame 6 - Particles far
    "\n"
    COLOR_RED "★" COLOR_YELLOW "⠀⠀⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀⠀⠀⠀⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀⠀⠀" COLOR_RED "★" COLOR_YELLOW "\n"
    "⠀⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀⠀⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀⠀\n"
    COLOR_CYAN "✦" COLOR_YELLOW "⠀⠀⠀⠀⠀" COLOR_RED "◉" COLOR_YELLOW "⠀⠀⠀⠀⠀" COLOR_CYAN "✦" COLOR_YELLOW "\n"
    "⠀⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀⠀⠀" COLOR_CYAN "★" COLOR_YELLOW "⠀⠀" COLOR_GREEN "✧" COLOR_YELLOW "⠀⠀\n"
    COLOR_RED "★" COLOR_YELLOW "⠀⠀⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀⠀⠀⠀⠀" COLOR_GREEN "✦" COLOR_YELLOW "⠀⠀⠀" COLOR_RED "★" COLOR_YELLOW "\n"
    COLOR_RESET,

    NULL
};

// ====================================================================================
// VICTORY ANIMATION - When device is compromised
// ====================================================================================

const char *victory_frames[] = {
    COLOR_GREEN
    "         ★\n"
    "        ╱ ╲\n"
    "       ╱   ╲\n"
    "      ╱ ✦   ╲\n"
    "     ╱       ╲\n"
    "    ╱    ✦    ╲\n"
    "   ╱___________╲\n"
    COLOR_YELLOW "   ▓▓▓▓▓▓▓▓▓▓▓▓▓\n" COLOR_RESET,
    
    COLOR_GREEN
    "         ✦\n"
    "        ╱ ╲\n"
    "       ╱ ★ ╲\n"
    "      ╱   ✦  ╲\n"
    "     ╱         ╲\n"
    "    ╱      ★    ╲\n"
    "   ╱_____________╲\n"
    COLOR_YELLOW "   ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\n" COLOR_RESET,
    
    NULL
};

// ====================================================================================
// THREAD-SAFE ANIMATION SYSTEM
// ====================================================================================

// Mutex for animation synchronization (prevent garbled output with multiple bots)
static pthread_mutex_t animation_mutex = PTHREAD_MUTEX_INITIALIZER;
static int animation_active = 0;

// Lock animation (returns 0 if locked successfully, -1 if another animation is running)
static int lock_animation() {
    pthread_mutex_lock(&animation_mutex);
    if (animation_active) {
        pthread_mutex_unlock(&animation_mutex);
        return -1;  // Another animation is playing
    }
    animation_active = 1;
    pthread_mutex_unlock(&animation_mutex);
    return 0;
}

// Unlock animation
static void unlock_animation() {
    pthread_mutex_lock(&animation_mutex);
    animation_active = 0;
    pthread_mutex_unlock(&animation_mutex);
}

// ====================================================================================
// ANIMATION FUNCTIONS
// ====================================================================================

// Show replication animation when bot replicates
void show_replication_animation() {
    if (lock_animation() != 0) {
        return;  // Skip if another animation is running
    }
    
    for (int i = 0; replication_frames[i] != NULL; i++) {
        CLEAR_SCREEN();
        printf("%s", replication_frames[i]);
        printf(COLOR_MAGENTA "\n    ⭐⭐⭐ BOT REPLICATING! ⭐⭐⭐" COLOR_RESET "\n");
        fflush(stdout);
        usleep(150000);  // 150ms per frame (faster explosion)
    }
    
    // Final message
    CLEAR_SCREEN();
    printf("\n\n");
    printf(COLOR_GREEN "         ╭────────────────────────────╮\n");
    printf(COLOR_GREEN "         │  " COLOR_YELLOW "⭐" COLOR_GREEN " NEW BOT ACTIVATED! " COLOR_YELLOW "⭐" COLOR_GREEN "  │\n");
    printf(COLOR_GREEN "         ╰────────────────────────────╯\n" COLOR_RESET);
    printf("\n");
    fflush(stdout);
    usleep(1000000);  // Show message for 1 second
    
    unlock_animation();
}

// Show animated splash screen
void show_splash_animation() {
    if (lock_animation() != 0) {
        return;  // Skip if another animation is running
    }
    
    for (int loop = 0; loop < 3; loop++) {  // 3 complete loops
        for (int i = 0; splash_frames[i] != NULL; i++) {
            CLEAR_SCREEN();
            printf("%s", splash_frames[i]);
            printf("\n");
            printf(COLOR_MAGENTA "                    Loading Higurashi System...\n" COLOR_RESET);
            printf(COLOR_CYAN "                          [");
            for (int j = 0; j < i * 5; j++) printf("█");
            for (int j = i * 5; j < 15; j++) printf("░");
            printf("]\n" COLOR_RESET);
            fflush(stdout);
            usleep(200000); // 200ms per frame
        }
    }
    
    CLEAR_SCREEN();
    printf("%s", higurashi_banner);
    printf("\n");
    sleep(2);
    
    unlock_animation();
}

// Show C2 splash
void show_c2_splash() {
    if (lock_animation() != 0) {
        return;  // Skip if busy
    }
    
    for (int i = 0; i < 3; i++) {
        CLEAR_SCREEN();
        if (i % 2 == 0) {
            printf(COLOR_MAGENTA);
        } else {
            printf(COLOR_CYAN);
        }
        printf("%s", higurama_banner);
        printf(COLOR_RESET);
        fflush(stdout);
        usleep(300000);
    }
    sleep(1);
    
    unlock_animation();
}

// Show spinner with message
void show_spinner(const char *message, int duration_ms) {
    pthread_mutex_lock(&animation_mutex);
    
    int frames = duration_ms / 100;
    for (int i = 0; i < frames; i++) {
        printf("\r%s %s", spinner_frames[i % 10], message);
        fflush(stdout);
        usleep(100000); // 100ms
    }
    printf("\r                                                    \r");
    fflush(stdout);
    
    pthread_mutex_unlock(&animation_mutex);
}

// Show victory animation
void show_victory() {
    pthread_mutex_lock(&animation_mutex);
    
    for (int loop = 0; loop < 3; loop++) {
        for (int i = 0; victory_frames[i] != NULL; i++) {
            printf("\r%s %s", victory_frames[i], kawaii_success);
            fflush(stdout);
            usleep(150000);
        }
    }
    printf("\n");
    
    pthread_mutex_unlock(&animation_mutex);
}

// Progress bar
void show_progress(const char *task, int current, int total) {
    pthread_mutex_lock(&animation_mutex);
    
    int bar_width = 40;
    float progress = (float)current / total;
    int filled = (int)(progress * bar_width);
    
    printf("\r%s" COLOR_CYAN "[", task);
    for (int i = 0; i < filled; i++) printf("█");
    for (int i = filled; i < bar_width; i++) printf("░");
    printf("] " COLOR_GREEN "%d%%" COLOR_RESET, (int)(progress * 100));
    fflush(stdout);
    
    pthread_mutex_unlock(&animation_mutex);
}

#endif // ASCII_ART_H
