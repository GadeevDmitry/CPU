#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <SFML/Graphics.hpp>

const int NUMBER_OF_FILES = 1000;
const int FILE_NAME_LEN   = 100;
const int WIDTH           = 960;
const int HEIGHT          = 720;

void get_filename(int img_cnt, char *const filename);

int main()
{
    FILE *stream = fopen("../tasks/video.asm", "w");
    assert(stream != nullptr);

    sf::RenderWindow wnd(sf::VideoMode(WIDTH, HEIGHT), "BAD");
    wnd.setFramerateLimit(60);

    while (wnd.isOpen())
    {
        for (int cnt = 1; cnt < NUMBER_OF_FILES; ++cnt)
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

            char filename[FILE_NAME_LEN] = "";
            get_filename(cnt, filename);

            unsigned int       code_int [WIDTH*HEIGHT] = {};
            unsigned long long code_long[WIDTH*HEIGHT] = {};

            sf::Image frame ;
            const sf::Uint8 *pixel;

            frame.loadFromFile(filename);
            pixel = frame.getPixelsPtr();

            for (int i = 0; i < WIDTH*HEIGHT; ++i)
            {
                code_int [i] = *(unsigned int     *) ((char *) pixel + 4*i);
                code_long[i] =  (unsigned long long) code_int[i];
                code_int [i] =  (unsigned int)      code_long[i];
            }

            sf::Texture tx;
            tx.create(WIDTH, HEIGHT);
            tx.update((sf::Uint8 *)code_int, WIDTH, HEIGHT, 0, 0);

            sf::Sprite sprite(tx);
            sprite.setPosition(0, 0);

            wnd.draw(sprite);
            wnd.display();
        }
    }
}

void get_filename(int img_cnt, char *const filename)
{
    assert (img_cnt > 0);

    if      (img_cnt < 10 ) sprintf(filename, "../img/img00%d.jpg", img_cnt);
    else if (img_cnt < 100) sprintf(filename, "../img/img0%d.jpg" , img_cnt);
    else                    sprintf(filename, "../img/img%d.jpg"  , img_cnt);
}
