#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

// Rozmiary okna i elementów GUI
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define CONSOLE_X 50
#define CONSOLE_Y 50
#define CONSOLE_WIDTH 700
#define CONSOLE_HEIGHT 400
#define BUTTON_WIDTH 150
#define BUTTON_HEIGHT 50
#define BUTTON_Y 500
#define ADD_BUTTON_X 50
#define REMOVE_BUTTON_X 250
#define EXIT_BUTTON_X 450

// Kolory przycisków
SDL_Color ADD_COLOR = {0, 255, 0, 255};    // Zielony
SDL_Color REMOVE_COLOR = {255, 0, 0, 255}; // Czerwony
SDL_Color EXIT_COLOR = {0, 0, 255, 255};   // Niebieski

// Bufor komunikatów
#define MAX_LOGS 100
char log_buffer[MAX_LOGS][256];
int log_count = 0;

// Funkcja dodająca komunikat do bufora
void add_log(const char *message) {
    if (log_count < MAX_LOGS) {
        snprintf(log_buffer[log_count], sizeof(log_buffer[log_count]), "%s", message);
        log_count++;
    } else {
        for (int i = 1; i < MAX_LOGS; i++) {
            snprintf(log_buffer[i - 1], sizeof(log_buffer[i - 1]), "%s", log_buffer[i]);
        }
        snprintf(log_buffer[MAX_LOGS - 1], sizeof(log_buffer[MAX_LOGS - 1]), "%s", message);
    }
}

// Funkcja do rysowania przycisków
void draw_button(SDL_Renderer *renderer, int x, int y, SDL_Color color, const char *text, TTF_Font *font) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect buttonRect = {x, y, BUTTON_WIDTH, BUTTON_HEIGHT};
    SDL_RenderFillRect(renderer, &buttonRect);

    SDL_Color textColor = {255, 255, 255, 255}; // Kolor tekstu (biały)
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_Rect textRect = {
        x + (BUTTON_WIDTH - textSurface->w) / 2,
        y + (BUTTON_HEIGHT - textSurface->h) / 2,
        textSurface->w,
        textSurface->h
    };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// Funkcja odczytująca nowe dane z bufora stdout
void read_stdout_logs(int stdout_pipe) {
    char buffer[256];
    ssize_t bytes_read;

    while ((bytes_read = read(stdout_pipe, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
	printf("Odczytano: %s\n", buffer); // Debugowanie: Wyświetl odczytane dane w konsoli
        add_log(buffer);
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0 || IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Symulacja Ula", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font = TTF_OpenFont("fonts/arial.ttf", 24);

    if (!window || !renderer || !font) {
        fprintf(stderr, "Initialization error: %s\n", SDL_GetError());
        return 1;
    }

    // Ładowanie tła
    SDL_Surface *backgroundSurface = IMG_Load("images/background.png");
    if (!backgroundSurface) {
        fprintf(stderr, "Błąd ładowania tła: %s\n", IMG_GetError());
        return 1;
    }
    SDL_Texture *backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
    SDL_FreeSurface(backgroundSurface);

    // Przekierowanie stdout do potoku
    int stdout_pipe[2];
    if (pipe(stdout_pipe) == -1) {
        perror("Błąd tworzenia potoku");
        return 1;
    }

    // Przekierowanie stdout
    dup2(stdout_pipe[1], STDOUT_FILENO);
    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK); // Odczyt nieblokujący

    SDL_Event e;
    bool gui_running = true;

    while (gui_running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                gui_running = false;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x = e.button.x;
                int y = e.button.y;

                // Kliknięcie "Dodaj ramki"
                if (x >= ADD_BUTTON_X && x <= ADD_BUTTON_X + BUTTON_WIDTH &&
                    y >= BUTTON_Y && y <= BUTTON_Y + BUTTON_HEIGHT) {
                    raise(SIGUSR1);
                }

                // Kliknięcie "Usuń ramki"
                if (x >= REMOVE_BUTTON_X && x <= REMOVE_BUTTON_X + BUTTON_WIDTH &&
                    y >= BUTTON_Y && y <= BUTTON_Y + BUTTON_HEIGHT) {
                    raise(SIGUSR2);
                }

                // Kliknięcie "Zakończ"
                if (x >= EXIT_BUTTON_X && x <= EXIT_BUTTON_X + BUTTON_WIDTH &&
                    y >= BUTTON_Y && y <= BUTTON_Y + BUTTON_HEIGHT) {
                    gui_running = false;
                }
            }
        }

        // Odczyt nowych logów
        read_stdout_logs(stdout_pipe[0]);

        // Rysowanie tła
        SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

        // Rysowanie białego kwadratu (konsola)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect consoleRect = {CONSOLE_X, CONSOLE_Y, CONSOLE_WIDTH, CONSOLE_HEIGHT};
        SDL_RenderFillRect(renderer, &consoleRect);

        // Wyświetlanie logów
        SDL_Color textColor = {0, 0, 0, 255}; // Czarny tekst
        int y_offset = CONSOLE_Y + 10;
        for (int i = 0; i < log_count; i++) {
            SDL_Surface *textSurface = TTF_RenderText_Solid(font, log_buffer[i], textColor);
            SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

            SDL_Rect textRect = {CONSOLE_X + 10, y_offset, textSurface->w, textSurface->h};
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
            y_offset += 30; // Odstęp między liniami
        }

        // Rysowanie przycisków
        draw_button(renderer, ADD_BUTTON_X, BUTTON_Y, ADD_COLOR, "Dodaj ramki", font);
        draw_button(renderer, REMOVE_BUTTON_X, BUTTON_Y, REMOVE_COLOR, "Usuń ramki", font);
        draw_button(renderer, EXIT_BUTTON_X, BUTTON_Y, EXIT_COLOR, "Zakończ", font);

        SDL_RenderPresent(renderer);
        SDL_Delay(100);
    }

    // Zwolnienie zasobów
    SDL_DestroyTexture(backgroundTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
