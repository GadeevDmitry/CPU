#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <SFML/Graphics.hpp>

const int NUMBER_OF_FILES = 1000;
const int FILE_NAME_LEN   = 100;
const int WIDTH           = 960;
const int HEIGHT          = 720;

void draw           (sf::RenderWindow *wnd, const sf::Uint8 *code);
void get_filename   (int img_cnt,           char *const filename );

int main()
{
    FILE *stream = fopen("../tasks/video.asm", "w");
    assert(stream != nullptr);

    sf::RenderWindow wnd(sf::VideoMode(WIDTH, HEIGHT), "BAD");
    wnd.setFramerateLimit(60);

    while (wnd.isOpen())
    {
        const sf::Uint8 *pixel_const = nullptr;

        unsigned int pixel_first [WIDTH*HEIGHT] = {};

        char filename[FILE_NAME_LEN] = "";
        get_filename(1, filename);

        sf::Image frame;
        frame.loadFromFile(filename);
        pixel_const = frame.getPixelsPtr();

        for (int i = 0; i < WIDTH*HEIGHT; ++i) pixel_first[i] = *(unsigned int *) ((char *)pixel_const + 4 * i);

        draw(&wnd, pixel_const);

        for (int cnt = 2; cnt < NUMBER_OF_FILES; ++cnt)
        {
            sf::Event event;

            while (wnd.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    wnd.close();
                    break;
                }
            }

            get_filename(cnt, filename);

            frame.loadFromFile(filename);
            pixel_const = frame.getPixelsPtr();

            for (int i = 0; i < WIDTH*HEIGHT; ++i)
            {
                if (pixel_first[i] == *(unsigned int *) ((char *)pixel_const + 4 * i)) continue;
                pixel_first[i] = *(unsigned int *) ((char *)pixel_const + 4 * i);
            }

            draw(&wnd, (sf::Uint8 *)pixel_first);
        }
    }
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
