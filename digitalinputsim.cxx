#include <atomic>
#include <chrono>
#include <vector>
#include <iostream>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "digitalinputsim.h"

// The thread-safe global word that holds the active pin states
std::atomic<uint32_t> gSimulatedDigitalWord(0);

struct DigitalInputSimulator::Impl {
    struct termios orig_termios;
    std::thread worker_thread;
    std::atomic<bool> running{false};

    std::vector<char> monitored_keys;
    char quit_character;

    void loop() {
        int currentLatchedMask = 0;

        while (running) {
            char ch;
            bool stateChanged = false;

            while (read(STDIN_FILENO, &ch, 1) > 0) {
                // Check if the user pressed the explicit quit key
                if (ch == quit_character) {
                    std::cout << "\n[Bela Mock Engine] Quit key detected. Initiating shutdown...\n";
                    running = false; // Gracefully breaks the while loop
                    return;
                }

                for (size_t pin = 0; pin < monitored_keys.size(); ++pin) {
                    if (ch == monitored_keys[pin]) {
                        currentLatchedMask ^= (1 << pin);
                        stateChanged = true;
                        break;
                    }
                }
            }

            if (stateChanged && running) {
                gSimulatedDigitalWord.store(static_cast<uint32_t>(currentLatchedMask));

                std::cout << "\r[Bela Mock Pins] ";
                for (size_t pin = 0; pin < monitored_keys.size(); ++pin) {
                    std::cout << "Pin" << pin << " (" << monitored_keys[pin] << "): "
                    << ((currentLatchedMask >> pin) & 1) << " | ";
                }
                std::cout << " (Press '" << quit_character << "' to quit) " << std::flush;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};

DigitalInputSimulator::DigitalInputSimulator(const std::string& targetKeys, char quitKey)
: pImpl(std::make_unique<Impl>())
{
    pImpl->quit_character = quitKey;
    for (char c : targetKeys) {
        pImpl->monitored_keys.push_back(c);
    }

    tcgetattr(STDIN_FILENO, &pImpl->orig_termios);
    struct termios raw = pImpl->orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

DigitalInputSimulator::~DigitalInputSimulator() {
    stop();
    tcsetattr(STDIN_FILENO, TCSANOW, &pImpl->orig_termios);
}

void DigitalInputSimulator::start() {
    pImpl->running = true;
    pImpl->worker_thread = std::thread(&DigitalInputSimulator::Impl::loop, pImpl.get());
}

void DigitalInputSimulator::stop() {
    if (pImpl->running) {
        pImpl->running = false;
    }
    if (pImpl->worker_thread.joinable()) {
        pImpl->worker_thread.join();
    }
}

void DigitalInputSimulator::join() {
    if (pImpl->worker_thread.joinable()) {
        pImpl->worker_thread.join(); // Blocks main cleanly until loop returns
    }
}
