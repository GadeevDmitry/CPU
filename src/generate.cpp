#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <SFML/Graphics.hpp>

const int NUMBER_OF_FILES = 1000;
const int FILE_NAME_LEN   = 100;
const int WIDTH           = 960;
const int HEIGHT          = 720;

void draw        (sf::RenderWindow *wnd, const sf::Uint8 *code);
void get_filename(int img_cnt,           char *const filename);

int main()
{
    sf::RenderWindow wnd(sf::VideoMode(WIDTH, HEIGHT), "BAD");
    wnd.setFramerateLimit(60);

    FILE *stream = fopen("../tasks/video.asm", "w");
    assert(stream != nullptr);

    char filename[FILE_NAME_LEN] = "";
    get_filename(1, filename);

    sf::Image frame;
    frame.loadFromFile(filename);
    const sf::Uint8 *pixels_const = frame.getPixelsPtr();

    sf::Uint8 *pixels_first  = (sf::Uint8 *) calloc(sizeof(int), HEIGHT * WIDTH);
    sf::Uint8 *pixels_second = (sf::Uint8 *) calloc(sizeof(int), HEIGHT * WIDTH);

    memcpy(pixels_first, pixels_const, 4 * WIDTH * HEIGHT);

    for (unsigned long long ram_cnt = 0; ram_cnt < WIDTH * HEIGHT; ++ram_cnt)
    {
        unsigned int color = *(unsigned int *) ((unsigned char *) pixels_first + 4 * ram_cnt);

        fprintf(stream, "push %llu \n", color  );
        fprintf(stream, "pop [%llu]\n", ram_cnt);
    }
    fprintf(stream, "draw\n");

    draw(&wnd, (sf::Uint8 *)pixels_first);

    for (int img_cnt = 2; img_cnt <= NUMBER_OF_FILES; ++img_cnt)
    {
        get_filename(img_cnt, filename);
        fprintf(stderr, "%s\n", filename);

        frame.loadFromFile(filename);

        // printf("%d\n%d\n", frame.getSize().x, frame.getSize().y);
        // return 0;

        pixels_const = frame.getPixelsPtr();
        memcpy(pixels_second, pixels_const, 4 * WIDTH * HEIGHT);

        for (unsigned long long ram_cnt = 0; ram_cnt < WIDTH * HEIGHT; ++ram_cnt)
        {
            unsigned int color_first  = *(unsigned int *) ((unsigned char *) pixels_first  + 4 * ram_cnt);
            unsigned int color_second = *(unsigned int *) ((unsigned char *) pixels_second + 4 * ram_cnt);

            if (color_first == color_second) continue;

            fprintf(stream, "push %llu \n", color_second);
            fprintf(stream, "pop [%llu]\n", ram_cnt     );
        }
        fprintf(stream, "draw\n");
        
        draw(&wnd, (sf::Uint8 *)pixels_second);
        
        memcpy(pixels_first, pixels_second, 4 * WIDTH * HEIGHT);
    }

   fclose(stream);
    return 0;
}

void draw(sf::RenderWindow *wnd, const sf::Uint8 *code)
{
    sf::Texture tx;
    tx.create(WIDTH, HEIGHT);
    tx.update(code, WIDTH, HEIGHT, 0, 0);

    sf::Sprite sprite(tx);
    sprite.setPosition(0, 0);

    (*wnd).draw(sprite);
    (*wnd).display();
}

void get_filename(int img_cnt, char *const filename)
{
    assert (img_cnt > 0);

    if      (img_cnt < 10 ) sprintf(filename, "../img/img00%d.jpg", img_cnt);
    else if (img_cnt < 100) sprintf(filename, "../img/img0%d.jpg" , img_cnt);
    else                    sprintf(filename, "../img/img%d.jpg"  , img_cnt);
}
