#ifndef DIGITALINPUTSIM_H
#define DIGITALINPUTSIM_H

#include <memory>
#include <string>

class DigitalInputSimulator {
public:
    // Configure keys to monitor AND the explicit quit character sequence
    DigitalInputSimulator(const std::string& targetKeys, char quitKey);
    ~DigitalInputSimulator();

    void start();
    void stop();
    void join(); // Expose thread joining to block main safely

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

#endif // DIGITALINPUTSIM_H
