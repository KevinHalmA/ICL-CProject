#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    EASY,
    NORMAL,
    HARD
} Difficulty;

typedef enum {
    RANDOM,
    TEXT
} Mode;

char *getRandomWord(Difficulty difficulty) {
    FILE* file;
    char* filename;
    int randomLine;
    int randomFileSelector;
    char word[128];
    int numLines = 0;


    //seed random number generator must be called in main program
    //generate random number for file selection
    randomFileSelector = rand() % 100;

    //choose file based on difficulty and random number
    switch(difficulty) {
        case EASY:
            if (randomFileSelector < 60) filename = "short.txt";
            else if (randomFileSelector < 90) filename = "medium.txt";
            else filename = "long.txt";
            break;
        case NORMAL:
            if (randomFileSelector < 25) filename = "short.txt";
            else if (randomFileSelector < 75) filename = "medium.txt";
            else filename = "long.txt";
            break;
        case HARD:
            if (randomFileSelector < 20) filename = "short.txt";
            else if (randomFileSelector < 50) filename = "medium.txt";
            else filename = "long.txt";
            break;
    }

    //open file
    file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s\n", filename);
        return EXIT_FAILURE;
    }

    //count lines
    while (fgets(word, sizeof(word), file) != NULL) {
        numLines++;
    }

    //get a random line number
    randomLine = rand() % numLines;

    //go to random line and get word
    rewind(file); //go back to start of file
    for (int i = 0; i <= randomLine; i++) {
        if (fgets(word, sizeof(word), file) == NULL) {
            //error or end of file
            fprintf(stderr, "%s\n", "randomLine error");
            fclose(file);
            return NULL;
        }
    }

    //remove newline character
    word[strcspn(word, "\n")] = 0;

    //close file
    fclose(file);

    //return a copy of the word (caller is responsible for freeing it)
    char *returnWord = strdup(word);
    if (returnWord == NULL){
        fprintf(stderr, "%s\n", "Memory alloc error for return value");
    }
    return returnWord;
}

// Default window dimensions
#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720

// Number of pages in the sidebar and the max length of each page name
#define NUM_PAGES 4
#define MAX_PAGE_CHARS 20

// Row and column spacing
#define ROW_SPACING 16
#define COL_SPACING 16

// Main margins
#define MAIN_LEFT_MARGIN 1
#define MAIN_TOP_MARGIN 4
#define MAIN_BOTTOM_MARGIN 4
#define MAIN_RIGHT_MARGIN 1

// Settings margins
#define SETTINGS_LEFT_MARGIN 2
#define SETTINGS_TOP_MARGIN 4
#define SETTINGS_BOTTOM_MARGIN 8
#define SETTINGS_RIGHT_MARGIN 8

// Represents a function that initialises a page grid
typedef void (*PagePtr)(GtkWidget *grid);

// Struct containing a page name and its associated grid initialisation function
typedef struct {
    char page[MAX_PAGE_CHARS];
    PagePtr func_ptr;
} PageEntry;

/**
 * Static functions that initialise the grid for each page in the sidebar menu
 */
static void add_typing_speed_tester(GtkWidget *);
static void add_settings(GtkWidget *);
static void add_login(GtkWidget *);
static void add_leaderboard(GtkWidget *);

/**
 * Table of page names and their associated grid initialisation function
 */
static PageEntry pageTable[] = {
    {"Typing speed tester", &add_typing_speed_tester},
    {"Settings", &add_settings},
    {"Login", &add_login},
    {"Leaderboard", &add_leaderboard},
};

#define NUM_SETTINGS 3
#define MAX_OPTIONS 5
#define NUM_DIFFICULTIES 3
#define NUM_NUM_WORDS 3
#define NUM_MODES 2

#define DEFAULT_DIFFICULTY "normal"
#define DEFAULT_NUM_WORDS "50"
#define DEFAULT_MODE "random"

typedef struct {
    int num_options;
    char options[MAX_OPTIONS][10];
    char label[15];
} SettingEntry;

static SettingEntry settings[] = {
    {NUM_DIFFICULTIES, {"easy", DEFAULT_DIFFICULTY, "hard"}, "Difficulty"},
    {NUM_NUM_WORDS, {"25", DEFAULT_NUM_WORDS, "100"}, "Word count"},
    {NUM_MODES, {DEFAULT_MODE, "text"}, "Mode"},
};

// Global declarations
GtkWidget *combos[NUM_SETTINGS];
GtkWidget *labels[6];
int correct = 0;
int wordcount = 0;
int running = 0;
clock_t start_t;

static Difficulty get_difficulty(GtkWidget *combo) {
    const char *diff = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    return atoi(diff);
}

static void on_entry_enter(GtkWidget *entry, GtkWidget *label) {
    const char *input = gtk_entry_get_text(GTK_ENTRY(entry));
    const char *goal = gtk_label_get_text(GTK_LABEL(label));
    char *start = "start";
    if (running == 0) {
        if (strcmp(start, input) == 0) {
            running = 1;
            start_t = clock();
            char *word = getRandomWord(get_difficulty(combos[0]));
            gtk_label_set_text(GTK_LABEL(label), word);
        }
    } else {
        clock_t end_t = clock();
        if (strcmp(input, goal) == 0) {
            correct++;
        }
        char *word = getRandomWord(get_difficulty(combos[0]));
        gtk_label_set_text(GTK_LABEL(label), word);
        wordcount++;
        char str[5];
        sprintf(str, "%d", wordcount);
        gtk_label_set_text(GTK_LABEL(labels[1]), str);
        float accuracy = ((float) correct / (float) wordcount) * 100;
        sprintf(str, "%2.2f%%", accuracy);
        gtk_label_set_text(GTK_LABEL(labels[3]), str);
        double elapsed = ((double) (end_t - start_t)) / CLOCKS_PER_SEC;
        sprintf(str, "%.2fs", elapsed);
        gtk_label_set_text(GTK_LABEL(labels[5]), str);
    }
    gtk_entry_set_text(GTK_ENTRY(entry), "");
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *header;
    GtkWidget *button;
    GtkWidget *image;
    GIcon *icon;
    GtkWidget *sidebar;
    GtkWidget *stack;
    GtkWidget *box;
    GtkWidget *widget;
    GtkWidget *grid;
    GtkWidget *combo;

    // Global initialising
    for (int i = 0; i < NUM_SETTINGS; i++) {
        combos[i] = gtk_combo_box_text_new();
    }
    for (int i = 0; i < 6; i++) {
        labels[i] = gtk_label_new("");
    }

    // Window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Typing Speed Tester 2 - electric boogaloo");
    gtk_window_set_default_size(GTK_WINDOW(window), DEFAULT_WIDTH, DEFAULT_HEIGHT);

    // Header bar
    header = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header), "Typing Speed Tester");
    gtk_window_set_titlebar(GTK_WINDOW(window), header);
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header), TRUE);

    button = gtk_button_new();
    icon = g_themed_icon_new("emblem-system-symbolic");
    image = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_BUTTON);
    g_object_unref(icon);
    gtk_container_add(GTK_CONTAINER(button), image);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header), button);

    // Sidebar menu and main UI
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    sidebar = gtk_stack_sidebar_new();
    gtk_box_pack_start(GTK_BOX(box), sidebar, FALSE, FALSE, 0);

    stack = gtk_stack_new();
    gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(sidebar), GTK_STACK(stack));

    widget = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(box), widget, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(box), stack, TRUE, TRUE, 0);

    // Loops through the pages in pageTable
    for (int i = 0; i < NUM_PAGES; i++) {
        // Initialises the grid
        grid = gtk_grid_new();
        gtk_grid_set_column_spacing(GTK_GRID(grid), COL_SPACING);
        gtk_grid_set_row_spacing(GTK_GRID(grid), ROW_SPACING);
        gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
        gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);

        // Calls the appropriate function for creating the page grid
        pageTable[i].func_ptr(grid);

        // Sets the page name on the sidebar menu
        gtk_stack_add_titled(GTK_STACK(stack), grid, pageTable[i].page, pageTable[i].page);
    }

    gtk_container_add (GTK_CONTAINER(window), box);
    gtk_widget_show_all(window);
}

static void add_typing_speed_tester(GtkWidget *grid) {
    GtkWidget *widget;
    GtkWidget *entry;
    int height = 3;
    int width = 1;

    srand(time(NULL));
    // Word popup
    widget = gtk_label_new("Type 'start' to begin.");
    gtk_grid_attach(GTK_GRID(grid), widget, MAIN_LEFT_MARGIN, MAIN_TOP_MARGIN, width, 1);

    // Entry text box
    entry = gtk_entry_new();
    g_signal_connect(entry, "activate", G_CALLBACK(on_entry_enter), widget);
    gtk_grid_attach(GTK_GRID(grid), entry, MAIN_LEFT_MARGIN, MAIN_TOP_MARGIN + 2, width, 1);

    gtk_label_set_text(GTK_LABEL(labels[0]), "Words Typed:");
    gtk_grid_attach(GTK_GRID(grid), labels[0], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), labels[1], 0, 1, 1, 1);
    gtk_label_set_text(GTK_LABEL(labels[2]), "Accuracy:");
    gtk_grid_attach(GTK_GRID(grid), labels[2], 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), labels[3], 0, 3, 1, 1);
    gtk_label_set_text(GTK_LABEL(labels[4]), "Time elapsed:");
    gtk_grid_attach(GTK_GRID(grid), labels[4], 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), labels[5], 0, 5, 1, 1);

    // Spacers
    // Left side padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, 0, 6, MAIN_LEFT_MARGIN, MAIN_TOP_MARGIN + height + MAIN_BOTTOM_MARGIN - 6);
    // Top padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, MAIN_LEFT_MARGIN, 0, width, MAIN_TOP_MARGIN);
    // Right side padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, MAIN_LEFT_MARGIN + width, 0, MAIN_RIGHT_MARGIN, MAIN_TOP_MARGIN + height + MAIN_BOTTOM_MARGIN);
}

static void add_settings(GtkWidget *grid) {
    GtkWidget *widget;
    GtkWidget *button;
    // Loop counter defined outside for use later
    int i = 0;
    // Width of box
    int width = 4;
    for (; i < NUM_SETTINGS; i++) {
        // Combo box
        for (int j = 0; j < settings[i].num_options; j++) {
            char id[5];
            sprintf(id, "%d", i);
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combos[i]), id, settings[i].options[j]);
        }
        gtk_combo_box_set_active(GTK_COMBO_BOX(combos[i]), 1);
        gtk_grid_attach(GTK_GRID(grid), combos[i], SETTINGS_LEFT_MARGIN, SETTINGS_TOP_MARGIN + i, width, 1);

        // Label
        widget = gtk_label_new(settings[i].label);
        gtk_grid_attach(GTK_GRID(grid), widget, 0, SETTINGS_TOP_MARGIN + i, SETTINGS_LEFT_MARGIN, 1);
    }

    // Spacers
    // Left side padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, 0, 0, SETTINGS_LEFT_MARGIN, SETTINGS_TOP_MARGIN + i + SETTINGS_BOTTOM_MARGIN);
    // Top padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, SETTINGS_LEFT_MARGIN, 0, width, SETTINGS_TOP_MARGIN);
    // Right side padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, SETTINGS_LEFT_MARGIN + width, 0, SETTINGS_RIGHT_MARGIN, SETTINGS_TOP_MARGIN + i + SETTINGS_BOTTOM_MARGIN);

    gtk_combo_box_new();
}

static void add_login(GtkWidget *grid) {
    GtkWidget *widget;
    GtkWidget *entry;
    int height = 2;
    int width = 1;

    // Word popup
    widget = gtk_label_new("Enter your name here, it will be recorded with your scores:");
    gtk_grid_attach(GTK_GRID(grid), widget, MAIN_LEFT_MARGIN, MAIN_TOP_MARGIN, width, 1);

    // Entry text box
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "name");
    gtk_grid_attach(GTK_GRID(grid), entry, MAIN_LEFT_MARGIN, MAIN_TOP_MARGIN + 1, width, 1);

    // Spacers
    // Left side padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, 0, 0, MAIN_LEFT_MARGIN, MAIN_TOP_MARGIN + height + MAIN_BOTTOM_MARGIN);
    // Top padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, MAIN_LEFT_MARGIN, 0, width, MAIN_TOP_MARGIN);
    // Right side padding
    widget = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), widget, MAIN_LEFT_MARGIN + width, 0, MAIN_RIGHT_MARGIN, MAIN_TOP_MARGIN + height + MAIN_BOTTOM_MARGIN);

}

static void add_leaderboard(GtkWidget *grid) {

}

// Main function - SHOULDN'T NEED CHANGING
int main (int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("uk.ac.ic.doc.type_tester", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}