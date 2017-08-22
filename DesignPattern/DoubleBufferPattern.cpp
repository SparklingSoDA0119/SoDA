// 7. Double Buffer Pattern
// 7.1 Example
class Framebuffer {
public :
    Framebuffer() { clear(); }
    void clear() {
        for (int i = 0; i < WIDTH * HEIGHT; ++i) {
            _pixels[i] = WHITE;
        }
    }

    void draw(int x, int y) {
        _pixels[(WIDTH * y) + x] = BLACK;
    }

    const char* getPixels() { return _pixels; }

private :
    static const int WIDTH = 160;
    static const int HEIGHT = 120;

    char _pixels[WIDTH * HEIGHT];
};

// 7.2 
class Scene {
public :
    void draw() {
        _buffer.clear();
        _buffer.draw(1, 1); _buffer.draw(4, 1);
        _buffer.draw(1, 3); _buffer.draw(2, 4);
        _buffer.draw(3, 4); _buffer.draw(4, 3);
    }

    Framebuffer& getBuffer() { return _buffer; }

private :
    Framebuffer _buffer;
};

// 7.3 Flickering solve
class Scene {
public :
    Scene() : 
        _current(&_buffers[0]), _next(&_buffers[1]) {}

    void draw() {
        _next->clear();
        _next->draw(1, 1);
        //...
        _next->draw(4, 3);
        swap();
    }

    Framebuffer& getBuffer() { return *_current; }

private :
    void swap() {
        Framebuffer* temp = _current;
        _current = _next;
        _next = temp;
    }

    Framebuffer _buffers[2];
    Framebuffer* _current;
    Framebuffer* _next;
};




