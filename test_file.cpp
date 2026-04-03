#include <iostream>

class LiquidGlass {
public:
    void render() {
        for (int i = 0; i < 10; ++i) {
            std::cout << "Rendering layer " << i << std::endl;
        }
    }
};

int main() {
    LiquidGlass lg;
    lg.render();
    return 0;
}
