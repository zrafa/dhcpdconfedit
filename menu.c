/*
 * menu.c - crea un menu a partir de un char *choices[]
 * 
 * Tome como ejemplo el codigo de la documentacion :
 * http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/menus.html
 */

#include <curses.h>
#include <menu.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4

static void print_in_middle(WINDOW *win, int sy, int sx, int w, char *st,
								 chtype color)
{	int len, x, y;
	float tmp;

	if(win == NULL)
		win = stdscr;

	getyx(win, y, x);
	if(sx != 0)
		x = sx;
	if(sy != 0)
		y = sy;
	if(w == 0)
		w = 80;

	len = strlen(st);
	tmp = (w - len)/ 2;
	x = sx + (int)tmp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", st);
	wattroff(win, color);
	refresh();
}

static char **get_choices_as_stringnumbers(int n)
{
	int i;
	char **r;

	r = malloc (sizeof(char *) * n);
        for(i = 0;i < n-1; ++i) {
		r[i] = malloc (3);
		sprintf(r[i], "%i", i);
		printf("nro=%s", r[i]);
	}
	r[n-1] = (char *)NULL;

	return r;
}

static void free_choices_as_stringnumbers(int n, char **r)
{
	int i;

        for(i = 0;i < n-1; ++i)
		free(r[i]);

	free(r);
}

static int process_events(WINDOW *menu_win, MENU *menu)
{
	int n = -1;
	int c;
	int quit_menu = 0;
	ITEM *cur;
	void (*p)(char *);

	while ( ! quit_menu ) {

		c = wgetch(menu_win);
		switch(c) {
		case KEY_DOWN:
			menu_driver(menu, REQ_DOWN_ITEM);
			break;
		case KEY_UP:
			menu_driver(menu, REQ_UP_ITEM);
			break;
		case KEY_NPAGE:
			menu_driver(menu, REQ_SCR_DPAGE);
			break;
		case KEY_PPAGE:
			menu_driver(menu, REQ_SCR_UPAGE);
			break;
		case 27: /* Enter */
			quit_menu = 1;
			break;
		case 10: /* Enter */

			cur = current_item(menu);
			p = item_userptr(cur);
			n = cur->index;
			printf("item : %s, index=%i\n", item_name(cur), cur->index);
			quit_menu = 1;
			break;
		default:
			break;

		}

                wrefresh(menu_win);
	}	

	return n;
}

static void settings_and_show(WINDOW *menu_win, MENU *menu)
{
	/* Los valores son para una terminal de 80x24 */

	/* Set main window and sub window */
        set_menu_win(menu, menu_win);
        set_menu_sub(menu, derwin(menu_win, 16, 60, 3, 3));
	set_menu_format(menu, 18, 1);
			
	/* Set menu mark to the string " * " */
        set_menu_mark(menu, " * ");

	/* Print a border around the main window and print a title */
        box(menu_win, 0, 0);
	print_in_middle(menu_win, 1, 0, 70, "Menu", COLOR_PAIR(1));
        
	/* Post the menu */
	post_menu(menu);
	wrefresh(menu_win);

	/* Mensajes al fondo de ayuda */
	attron(COLOR_PAIR(2));
	mvprintw(LINES - 2, 0, "Utilice las teclas de flecha o PageUp/PageDown para seleccionar un item");
	mvprintw(LINES - 1, 0, "ENTER para Aceptar el item - ESCape para Salir");
	attroff(COLOR_PAIR(2));
	refresh();

}

/*
 * Dibuja y controla el menu 
 * TODO: quitar e independizar la seccion de inicializacion de curses
 */
int menu_main(char **choices, int n_choices)
{	
	ITEM **items;
	MENU *menu;
	WINDOW *menu_win;
	int i, n;
	char **choices2;
		
	/* Initialize curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_CYAN, COLOR_BLACK);
		
	/* Create items */
	choices2 = get_choices_as_stringnumbers(n_choices);
	items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
	for(i = 0; i < n_choices; ++i)
		items[i] = new_item(choices2[i], choices[i]);

	/* Crate menu */
	menu = new_menu((ITEM **)items);

	/* Create the window to be associated with the menu */
	menu_win = newwin(20, 70, 2, 10);
	keypad(menu_win, TRUE);
     
	settings_and_show(menu_win, menu);

	n = process_events(menu_win, menu);
	printf("item :  index=%i\n", n);
	sleep (3);

	/* Unpost and free all the memory taken up */
	unpost_menu(menu);
	free_menu(menu);
	for(i = 0; i < n_choices; ++i)
		free_item(items[i]);
	free_choices_as_stringnumbers(n_choices, choices2);
	endwin();

	return n;
}

/*
 * Ejemplo de uso
 * int main (void)
 * {
 * 	int n = 12;
 * 	char *choices[] = {
 * 		"Choice 1",
 * 		"Choice 2",
 * 		"Choice 3",
 * 		"Choice 4",
 * 		"Choice 5",
 * 		"Choice 6",
 * 		"Choice 7",
 * 		"Choice 8",
 * 		"Choice 9",
 * 		"Guardar los Cambios Efectuados",
 * 		"Exit",
 * 		(char *)NULL,
 * 	};
 * 
 * 	menu_main(choices, n);
 * 	menu_main(choices, n);
 * 
 *	return 0;
 * }
 */
