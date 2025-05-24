#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "tinyexpr.h"

#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 600
#define MAX_BUFFER 512

#define STYLE_CSS_PATH "style/style.css"
#define STYLE_CSS_CLEAR "buttons-clear"
#define STYLE_CSS_OPERATOR "buttons-operator"
#define STYLE_CSS_NUMBER "buttons-number"

typedef struct {
  GtkWidget *scrolled;
  GtkWidget *list_box;
} ST_CalcHistory;

typedef struct {
  char *label;
  int col;
  int row;
  int width;
  int height;
  char *style;
  GtkWidget *widget;
} ST_Button;

typedef struct {
  GtkWidget *window;
  GtkWidget *box;
  ST_CalcHistory history;
  GtkWidget *entry;
  GtkWidget *grid;
  ST_Button button[17];
  bool clear;
} ST_AppUI;

static void create_main_window(GtkApplication *app, gpointer data);
static void initialize_calc_history(ST_AppUI *app_ui, GtkWidget *box, ST_CalcHistory *history);
static void initialize_buttons(ST_AppUI *app_ui, GtkWidget *grid, ST_Button *button);
static void selected_history_row(GtkListBox *list_box, GtkListBoxRow *row, gpointer data);
static void process_button(GtkButton *button, gpointer data);
static void calculate(ST_AppUI *app_ui, GtkEntryBuffer *buffer, const char *expression);
static void add_to_history(ST_AppUI *app_ui, const char *expression, const char *result);
static gboolean key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer data);
int count_digits(int num);
static gboolean timeout(gpointer data);

int main (int argc, char **argv){
  GtkApplication *app;
  ST_AppUI app_ui;
  app_ui.clear = false;
  int status;

  app = gtk_application_new("org.gtk.calculator", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app, "activate", G_CALLBACK(create_main_window), &app_ui);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  return status;
}

static void create_main_window(GtkApplication *app, gpointer data){
  ST_AppUI *app_ui = (ST_AppUI *)data;

  app_ui->window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(app_ui->window), "Calculator");
  gtk_window_set_default_size(GTK_WINDOW(app_ui->window), WINDOW_WIDTH, WINDOW_HEIGHT);
  gtk_window_set_resizable(GTK_WINDOW(app_ui->window), false);
  gtk_widget_add_css_class(app_ui->window, "calculator");
  
  app_ui->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_add_css_class(app_ui->box, "calculator");
  gtk_window_set_child(GTK_WINDOW(app_ui->window), app_ui->box);

  initialize_calc_history(app_ui, app_ui->box, &app_ui->history);

  app_ui->entry = gtk_entry_new();
  gtk_editable_set_editable(GTK_EDITABLE(app_ui->entry), TRUE);
  gtk_widget_set_size_request(app_ui->entry, -1, 50);
  gtk_widget_add_css_class(app_ui->entry, "value");
  gtk_box_append(GTK_BOX(app_ui->box), app_ui->entry);

  GtkEventController *controller = gtk_event_controller_key_new();
  gtk_event_controller_set_propagation_phase(controller, GTK_PHASE_CAPTURE);
  g_signal_connect(controller, "key-pressed", G_CALLBACK(key_pressed), app_ui);
  gtk_widget_add_controller(app_ui->window, controller);

  app_ui->grid = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(app_ui->grid), 3);
  gtk_grid_set_row_spacing(GTK_GRID(app_ui->grid), 3);
  gtk_widget_add_css_class(app_ui->grid, "buttons-container");
  gtk_box_append(GTK_BOX(app_ui->box), app_ui->grid);
  
  initialize_buttons(app_ui, app_ui->grid, app_ui->button);

  GtkCssProvider *provider = gtk_css_provider_new();
  GFile *css_file = g_file_new_for_path(STYLE_CSS_PATH);
  gtk_css_provider_load_from_file(provider, css_file);
  gtk_style_context_add_provider_for_display(gdk_display_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  
  GtkRoot *root = gtk_widget_get_root(app_ui->window);
  gtk_root_set_focus(root, app_ui->entry);

  gtk_window_present(GTK_WINDOW(app_ui->window));
}

static void initialize_calc_history(ST_AppUI *app_ui, GtkWidget *box, ST_CalcHistory *history){
  history->scrolled = gtk_scrolled_window_new();

  history->list_box = gtk_list_box_new();
  gtk_widget_add_css_class(history->list_box, "scrolled");
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(history->list_box), GTK_SELECTION_SINGLE);
  g_signal_connect(history->list_box, "row-activated", G_CALLBACK(selected_history_row), app_ui);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(history->scrolled), history->list_box);
  gtk_widget_set_size_request(history->scrolled, -1, 200);

  gtk_box_append(GTK_BOX(box), history->scrolled);
}

static void initialize_buttons(ST_AppUI *app_ui, GtkWidget *grid, ST_Button *button){
  const ST_Button list[] = {
    {"C", 0, 0, 3, 1, STYLE_CSS_CLEAR, NULL},
    {"/", 3, 0, 1, 1, STYLE_CSS_OPERATOR, NULL},
    
    {"7", 0, 1, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"8", 1, 1, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"9", 2, 1, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"x", 3, 1, 1, 1, STYLE_CSS_OPERATOR, NULL},
    
    {"4", 0, 2, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"5", 1, 2, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"6", 2, 2, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"-", 3, 2, 1, 1, STYLE_CSS_OPERATOR, NULL},

    {"1", 0, 3, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"2", 1, 3, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"3", 2, 3, 1, 1, STYLE_CSS_NUMBER, NULL},
    {"+", 3, 3, 1, 1, STYLE_CSS_OPERATOR, NULL},

    {"0", 0, 4, 2, 1, STYLE_CSS_NUMBER, NULL},
    {".", 2, 4, 1, 1, STYLE_CSS_OPERATOR, NULL},
    {"=", 3, 4, 1, 1, STYLE_CSS_OPERATOR, NULL},
  };

  memcpy(button, list, sizeof(list));

  for(int i = 0; i < 17; i++){
      button[i].widget = gtk_button_new_with_label(button[i].label);
      
      gtk_grid_attach(GTK_GRID(grid), button[i].widget, button[i].col, button[i].row, button[i].width, button[i].height);

      gtk_widget_add_css_class(button[i].widget, button[i].style);
      
      gtk_widget_set_hexpand(button[i].widget, true);
      gtk_widget_set_vexpand(button[i].widget, true);
      g_signal_connect(button[i].widget, "clicked", G_CALLBACK(process_button), app_ui);
    }
}

static void selected_history_row(GtkListBox *list_box, GtkListBoxRow *row, gpointer data){
  ST_AppUI *app_ui = (ST_AppUI *)data;

  GtkWidget *child = gtk_list_box_row_get_child(row);

  if(GTK_IS_BOX(child)){
    GtkWidget *label = gtk_widget_get_first_child(child);
      if(GTK_IS_LABEL(label)){
        GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(app_ui->entry));
        
        const char *text = gtk_label_get_text(GTK_LABEL(label));
        gtk_entry_buffer_set_text(buffer, text, -1);
      }
    }
  gtk_list_box_unselect_row(list_box, row);
}

static void process_button(GtkButton *button, gpointer data){
  ST_AppUI *app_ui = (ST_AppUI *)data;

  const char *label = gtk_button_get_label(button);

  GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(app_ui->entry));
  const char *input = gtk_entry_buffer_get_text(buffer);
  
  if(app_ui->clear == true){
    gtk_entry_buffer_set_text(buffer, "", -1);
    app_ui->clear = false;
  }

  if(strcmp("=", label) == 0){
    calculate(app_ui, buffer, input);
    app_ui->clear = true;
  }else if(strcmp("C", label) == 0){
    gtk_entry_buffer_set_text(buffer, "", -1);
  }else {
    size_t length = strlen(label) + strlen(input) + 1;
    char *new_input = malloc(length);
    if(new_input == NULL){
      return;
    }
    snprintf(new_input, length, "%s%s", input, label);
    gtk_entry_buffer_set_text(buffer, new_input, -1);
    
    free(new_input);
    new_input = NULL;
  }
}

static void calculate(ST_AppUI *app_ui, GtkEntryBuffer *buffer, const char *expression){
  int error;
  double result = te_interp(expression, &error);
    
  char output[MAX_BUFFER];

  if(result == (int)result){
    snprintf(output, MAX_BUFFER, "%d", (int)result);
  }else {
    snprintf(output, MAX_BUFFER, "%.6g", result);
  }

  if(error == 0 && strcmp(output, "nan") != 0 && g_utf8_validate(output, -1, NULL)){
    char *valid_output = g_utf8_make_valid(output, -1);

    add_to_history(app_ui, expression, valid_output);

    gtk_entry_buffer_set_text(buffer, valid_output, -1);
  }else {
    gtk_entry_buffer_set_text(buffer, "Expression error", -1);
    
    g_timeout_add(1000, timeout, NULL);

    gtk_entry_buffer_set_text(buffer, "", -1);
  }
}


static void add_to_history(ST_AppUI *app_ui, const char *expression, const char *result){
  GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  
  GtkWidget *label = gtk_label_new(expression);
  gtk_widget_set_size_request(label, 100, -1);
  gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
  gtk_widget_set_hexpand(label, true);
  gtk_widget_add_css_class(label, "history");
  gtk_box_append(GTK_BOX(box), label);

  label = gtk_label_new("=");
  gtk_widget_set_size_request(label, 25, -1);
  gtk_widget_set_hexpand(label, true);
  gtk_widget_add_css_class(label, "history");
  gtk_box_append(GTK_BOX(box), label);

  label = gtk_label_new(result);
  gtk_widget_set_size_request(label, 75, -1);
  gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
  gtk_widget_set_hexpand(label, true);
  gtk_widget_add_css_class(label, "history");
  gtk_box_append(GTK_BOX(box), label);

  gtk_list_box_append(GTK_LIST_BOX(app_ui->history.list_box), box);
}

static gboolean key_pressed(GtkEventControllerKey *controller, guint keyval, guint keycode, GdkModifierType state, gpointer data){
  (void)controller;
  (void)keycode;
  (void)state;

  ST_AppUI *app_ui = (ST_AppUI *)data;

  GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(app_ui->entry));
  const char *input = gtk_entry_buffer_get_text(buffer);
  
  if(app_ui->clear == true){
    gtk_entry_buffer_set_text(buffer, "", -1);
    app_ui->clear = false;
  }

  if(keyval == GDK_KEY_Return || keyval == GDK_KEY_KP_Enter || keyval == GDK_KEY_equal){
    calculate(app_ui, buffer, input); 
    app_ui->clear = true;
    return true;
  }else if(keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9){
    int num = keyval - GDK_KEY_0;
    
    size_t length = count_digits(num) + strlen(input) + 1;
    char *new_input = malloc(length);
    if(new_input == NULL){
      return false;
    }
    
    snprintf(new_input, length, "%s%d", input, num);
    gtk_entry_buffer_set_text(buffer, new_input, -1);
    
    free(new_input);
    new_input = NULL;

    return true;
  }else if(keyval == GDK_KEY_plus || keyval == GDK_KEY_minus || keyval == GDK_KEY_asterisk || keyval == GDK_KEY_slash){
    char operator = (char)keyval;
    
    size_t length = 1 + strlen(input) + 1;
    char *new_input = malloc(length);
    if(new_input == NULL){
      return false;
    }

    snprintf(new_input, length, "%s%c", input, operator);
    gtk_entry_buffer_set_text(buffer, new_input, -1);

    free(new_input);
    new_input = NULL;

    return true;
  }else if(keyval == GDK_KEY_BackSpace){
    int length = gtk_entry_buffer_get_length(buffer);

    gtk_entry_buffer_delete_text(buffer, length - 1, 1);

    return true;
  }
  return false;
}

int count_digits(int num){
  int counter = 0;
  while(num != 0){
    num = num / 10;
    counter++;
  }
  return counter;
}

static gboolean timeout(gpointer data){
  (void)data;
  return false;
}
