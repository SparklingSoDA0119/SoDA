// 5. Singletone Pattern
// 5.1 It guarantee having only one instance
// 5.2 It provide global approach point
// 5.2.1
class FileSystem {
public :
    static FileSystem& instance() {
        if (_instance == NULL) {
            _instance = new FileSystem();
        }

        return *_instance;
    }

private :
    FileSystem() {}
    static FileSystem* _instance;
};

// 5.2.2 trend
class FileSystem {
public :
    static FileSystem& instance() {
        static FileSystem* instance = new FileSystem();
        return *instance;
    }
    
private :
    FileSystem() {};
};

// 5.3 Why do use sigleton?
// 5.3.1 If you don't use, instance is not created.
// 5.3.2 It is initialized at Run-time
// 5.3.3 It can be inheritanced.
// 5.3.3.1 First, create parent class
class FileSystem {
public :
    virtual ~FileSystem() {}
    virtual char* readFile(char* path) = 0;
    virtual void writeFile(char* path, char* contents) = 0;
};

// 5.3.3.2 Define subclass each platform
class PS3FileSystem : public FileSystem {
public :
    virtual char* readFile(char* path) {
        // 
    }
    virtual void writeFile(char* path, char* contents) {
    }
};

// .. more

// 5.3.3.3 It is part of creating instance
FileSystem& FileSystem::instance() {
#if PLATFORM == PLAYSTATION3
    static FileSystem* instance = new PS3FileSystem();
#elif ...

#endif
    return *instance;
}

// 5.4 Only one instance
class FileSystem {
public :
    FileSystem() {
        assert(!_instantiated);
        _instantiated = true;
    }
    
    ~FileSystem() {
        _instantiated = false;
    }

private :
    static bool _instantiated;
};

bool FileSystem::_instantiated = false;


// 5.5 To approach to instance easyly.
// 5.5.1 throughput
// 5.5.2 Geting for parents class
class GameObject {
protected :
    Log& getLog() { return  _log; }

private :
    static Log& log;
};

class Enemy : public GameObject {
    void doSomething() {
        getLog().write("I can Log!");
    }
};

// 5.5.3 Getting global object
// 5.5.3.1
class Game {
public :
    static Game& instance() { return _instance; }

    Log& getLog() { return *_log; }
    FileSystem& getFileSystem() { return *fileSystem; }
    AudioPlayer& getAudioPlayer() { return *audioPlayer; }

private :
    static Game _instance;
    Log* _log;
    FileSystem* _fileSystem;
    AudioPlayer* _audkoPlayer;
};

// 5.5.3.2 Calling Function 
Game::instance().getAudioPlayer().play(MUSIC_3));

