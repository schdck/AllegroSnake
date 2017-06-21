#include <stdio.h>
#include <stdlib.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

/* Compiling command */ 
/*

g++ -o snake.app main.c -I/usr/include/allegro5 -L/usr/lib -lallegro -lallegro_primitives -lallegro_font -lallegro_ttf

*/

/* Defines */

#define     SCREEN_WIDTH        800
#define     SCREEN_HEIGHT       600
#define     SCREEN_FPS          60

#define     SNAKE_START_LENGTH  5
#define     SNAKE_SIZE          20

#define     DIRECTION_NULL      0
#define     DIRECTION_T         1
#define     DIRECTION_B         2
#define     DIRECTION_R         3
#define     DIRECTION_L         4

#define     QUEUE_MAX_SIZE      3
#define     QUEUE_ELEMENT_TYPE  char

/* Structs declaration */

struct POINT {
    int x;
    int y;
};

struct SNAKE {
    POINT points[(int)((SCREEN_WIDTH / SNAKE_SIZE) * (SCREEN_HEIGHT / SNAKE_SIZE))];
    int size;
};

struct QUEUE {
    int length;
    char elements[QUEUE_MAX_SIZE];
};

/* Queue functions */

QUEUE_ELEMENT_TYPE GetFirst(QUEUE *queue)
{
    QUEUE_ELEMENT_TYPE element = queue->elements[0];

    queue->length--;

    for(int i = 0; i < queue->length; i++)
    {
        queue->elements[i] = queue->elements[i + 1];
    }

    return element;
}

QUEUE_ELEMENT_TYPE LookFirst(QUEUE *queue)
{
    return queue->elements[0];
}

void Enfilerate(QUEUE *QUEUE, QUEUE_ELEMENT_TYPE element)
{
    if(QUEUE->length >= QUEUE_MAX_SIZE)
    {
        return;
    }

    QUEUE->elements[QUEUE->length++] = element;
}

/* Other functions */

POINT* generate_random_point_for_apple(SNAKE *snake)
{
    POINT *p = (POINT *) malloc(sizeof(POINT));
    
    bool exists;

    do {
        exists = false;

        p->x = ((random() % SCREEN_WIDTH) / SNAKE_SIZE) * SNAKE_SIZE;
        p->y = ((random() % SCREEN_HEIGHT) / SNAKE_SIZE) * SNAKE_SIZE;

        for(int i = 0; i < snake->size; i++)
        {

            if(snake->points[i].x == p->x && snake->points[i].y == p->y)
            {
                exists = true;
            }
        }
    } while(exists);   

    return p;
}

void copy_point(POINT *destination, POINT source)
{
    destination->x = source.x;
    destination->y = source.y;
}

void move_snake(SNAKE *snake, int direction)
{
    for(int i = snake->size - 1; i > 0; i--)
    {
        copy_point(&snake->points[i], snake->points[i - 1]);
    }

    switch(direction)
    {
        case DIRECTION_T:
            snake->points[0].y -= SNAKE_SIZE;
            break;

        case DIRECTION_B:
            snake->points[0].y += SNAKE_SIZE;
            break;

        case DIRECTION_R:
            snake->points[0].x += SNAKE_SIZE;
            break;

        case DIRECTION_L:
            snake->points[0].x -= SNAKE_SIZE;
            break;
    }

    if(snake->points[0].x < 0)
    {
        snake->points[0].x = SCREEN_WIDTH - SNAKE_SIZE;
    }
    else if(snake->points[0].x >= SCREEN_WIDTH)
    {
        snake->points[0].x = 0;
    }
    else if(snake->points[0].y < 0)
    {
        snake->points[0].y = SCREEN_HEIGHT - SNAKE_SIZE;
    }
    else if(snake->points[0].y >= SCREEN_HEIGHT)
    {
        snake->points[0].y = 0;
    }
}

void draw_snake(SNAKE *snake)
{
    al_clear_to_color(al_map_rgb(0, 0, 0));

    for(int i = 0; i < snake->size; i++)
    {
        al_draw_filled_rectangle(snake->points[i].x, snake->points[i].y, snake->points[i].x + SNAKE_SIZE, snake->points[i].y + SNAKE_SIZE, al_map_rgba(50, 150, 50, 20));
    }
}

void insert_point(SNAKE *snake)
{
    POINT *last = &snake->points[snake->size - 1],
          *previous = &snake->points[snake->size - 2],
          *current = &snake->points[snake->size];

    if(last->x > previous->x)
    {
        current->x = last->x + SNAKE_SIZE;
        current->y = last->y;
    }
    else if(last->x < previous->x)
    {
        current->x = last->x - SNAKE_SIZE;
        current->y = last->y;
    }
    else if(last->y > previous->y)
    {
        current->y = last->y + SNAKE_SIZE;
        current->x = last->x;
    }
    else
    {
        current->y = last->y - SNAKE_SIZE;
        current->x = last->x;
    }

    snake->size++;
}

void initialize_addons()
{
    if(!al_init()) 
    {   
        fprintf(stderr, "Error initializing allegro5\n");
        exit(-1);
    }

    if(!al_init_font_addon())
    {
        fprintf(stderr, "Error initializing font addon\n");
        exit(-1);
    }

    if(!al_init_ttf_addon())
    {
        fprintf(stderr, "Error initializing ttf addont\n");
        exit(-1);
    }

    if(!al_init_primitives_addon())
    {
        fprintf(stderr, "Error initializing primitives addon\n");
        exit(-1);
    }
}

SNAKE *create_snake()
{
    SNAKE *snake = (SNAKE *) malloc(sizeof(SNAKE));

    
    snake->size = 1;
    snake->points[0].x = SCREEN_WIDTH / 2;
    snake->points[0].y = SCREEN_HEIGHT / 2;

    for(int i = 0; i < SNAKE_START_LENGTH; i++)
    {
        insert_point(snake);
    }

    return snake;
}

/* Global variables */

QUEUE *_keyboard_event_queue = (QUEUE *) malloc(sizeof(QUEUE));

int main()
{
    QUEUE_ELEMENT_TYPE current_direction = DIRECTION_R;
    ALLEGRO_DISPLAY *display = NULL;
    ALLEGRO_EVENT_QUEUE *event_queue = NULL;
    ALLEGRO_FONT *game_font = NULL;;
    SNAKE *snake = NULL;
    POINT *apple = NULL, plus_ten_pos;

    initialize_addons();

    if(!(game_font = al_load_font("./font/calibri.ttf", 16, 1)))
    {
        fprintf(stderr, "Error loading font\n");
        exit(-1);
    }

    if(!(display = al_create_display(800, 600))) 
    {
        fprintf(stderr, "Error creating main display\n");
        exit(-1);
    }

    if(!(event_queue = al_create_event_queue())) 
    {
        fprintf(stderr, "Error creating event queue\n");
        al_destroy_display(display);
        exit(-1);
    }

    al_install_keyboard();
    al_set_window_title(display, "AllegroSnake by github.com/schdck");

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());
    
    snake = create_snake();

    apple = generate_random_point_for_apple(snake);

    bool redirected = false, game_over = false;
    double fps_start = al_get_time(), fps_end;
    int frames, speed_counter = 0, plus_ten_counter;

    while(!game_over)
    {
        ALLEGRO_EVENT event;

        if(al_get_next_event(event_queue, &event))
        {
            if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            {
                game_over = true;
            }
            else if(event.type == ALLEGRO_EVENT_KEY_UP && !redirected)
            {
                QUEUE_ELEMENT_TYPE direction = 0, prev_direction = _keyboard_event_queue->length > 0 ? LookFirst(_keyboard_event_queue) : current_direction;

                if((event.keyboard.keycode == ALLEGRO_KEY_W || event.keyboard.keycode == ALLEGRO_KEY_UP) && prev_direction != DIRECTION_B)
                {
                    direction = DIRECTION_T;
                }
                else if((event.keyboard.keycode == ALLEGRO_KEY_S || event.keyboard.keycode == ALLEGRO_KEY_DOWN) && prev_direction != DIRECTION_T)
                {
                    direction = DIRECTION_B;
                }
                else if((event.keyboard.keycode == ALLEGRO_KEY_D || event.keyboard.keycode == ALLEGRO_KEY_RIGHT) && prev_direction != DIRECTION_L)
                {
                    direction = DIRECTION_R;
                }
                else if((event.keyboard.keycode == ALLEGRO_KEY_A || event.keyboard.keycode == ALLEGRO_KEY_LEFT) && prev_direction != DIRECTION_R)
                {
                    direction = DIRECTION_L;
                }

                if(direction > DIRECTION_NULL)
                {
                    Enfilerate(_keyboard_event_queue, direction);
                }
            }
        }

        draw_snake(snake);

        if(++speed_counter >= SCREEN_FPS * 1.3)
        {
            if(_keyboard_event_queue->length > 0)
            {
                current_direction = GetFirst(_keyboard_event_queue);
            }

            move_snake(snake, current_direction);

            speed_counter = 0;
        }

        if(snake->points[0].x == apple->x && snake->points[0].y == apple->y)
        {
            copy_point(&plus_ten_pos, *apple);
            plus_ten_counter = 120;

            apple = generate_random_point_for_apple(snake);

            insert_point(snake);    
        }

        if(--plus_ten_counter >= 0)
        {
            al_draw_text(game_font, al_map_rgb(255, 255, 255), plus_ten_pos.x - SNAKE_SIZE, plus_ten_pos.y - SNAKE_SIZE, 0, "+10");
        }

        al_draw_filled_rectangle(apple->x, apple->y, apple->x + SNAKE_SIZE, apple->y + SNAKE_SIZE, al_map_rgb(255, 0, 0));

        for(int i = 0; i < snake->size; i++)
        {
            for(int j = 0; j < snake->size; j++)
            {
                if(i == j)
                {
                    j++;
                }

                if(snake->points[i].x == snake->points[j].x && snake->points[i].y == snake->points[j].y)
                {
                    al_draw_filled_rectangle(snake->points[i].x, snake->points[i].y, snake->points[i].x + SNAKE_SIZE, snake->points[i].y + SNAKE_SIZE, al_map_rgb(255, 255, 255));
                    al_flip_display();
                    al_rest(1.5);

                    game_over = true;
                }
            }
        }

        if(!game_over)
        {
            POINT head = snake->points[0];
            
            int average_x = head.x + SNAKE_SIZE / 2,
                average_y = head.y + SNAKE_SIZE / 2;

            switch(current_direction)
            {
                case DIRECTION_T:
                    al_draw_filled_rectangle(average_x - 2, head.y, average_x + 2, head.y - 4, al_map_rgb(255, 0, 0));

                    al_draw_filled_rectangle(average_x - 6, head.y + 2, average_x - 3, head.y + 5, al_map_rgb(0, 0, 0));
                    al_draw_filled_rectangle(average_x + 6, head.y + 2, average_x + 3, head.y + 5, al_map_rgb(0, 0, 0));
                    break;
                case DIRECTION_B:
                    al_draw_filled_rectangle(average_x - 2, head.y + SNAKE_SIZE, average_x + 2, head.y + SNAKE_SIZE + 4, al_map_rgb(255, 0, 0));

                    al_draw_filled_rectangle(average_x - 6, head.y - 2 + SNAKE_SIZE, average_x - 3, head.y - 5 + SNAKE_SIZE, al_map_rgb(0, 0, 0));
                    al_draw_filled_rectangle(average_x + 6, head.y - 2 + SNAKE_SIZE, average_x + 3, head.y - 5 + SNAKE_SIZE, al_map_rgb(0, 0, 0));
                    break;
                case DIRECTION_L:
                    al_draw_filled_rectangle(head.x, average_y - 2, head.x - 4, average_y + 2, al_map_rgb(255, 0, 0));

                    al_draw_filled_rectangle(head.x + 2, average_y - 6, head.x + 5, average_y - 3, al_map_rgb(0, 0, 0));
                    al_draw_filled_rectangle(head.x + 2, average_y + 6, head.x + 5, average_y + 3, al_map_rgb(0, 0, 0));
                    break;
                case DIRECTION_R:
                    al_draw_filled_rectangle(head.x + SNAKE_SIZE, average_y - 2, head.x + SNAKE_SIZE + 4, average_y + 2, al_map_rgb(255, 0, 0));

                    al_draw_filled_rectangle(head.x - 2 + SNAKE_SIZE, average_y - 6, head.x - 5 + SNAKE_SIZE, average_y - 3, al_map_rgb(0, 0, 0));
                    al_draw_filled_rectangle(head.x - 2 + SNAKE_SIZE, average_y + 6, head.x - 5 + SNAKE_SIZE, average_y + 3, al_map_rgb(0, 0, 0));
                    break;
            }
        }

        al_flip_display();

        if(++frames > SCREEN_FPS)
        {
            frames = 0;

            al_rest(1 - fps_end - fps_start);

            fps_start = al_get_time();
        }
    }

    fprintf(stderr, "\nGAME OVER!\nYou made %d points.\n\n", snake->size * 10 - SNAKE_START_LENGTH * 10 - 10);

    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_font(game_font);

    free(snake);
    free(apple);
}