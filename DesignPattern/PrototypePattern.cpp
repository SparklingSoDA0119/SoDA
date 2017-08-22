// 4. Prototype Pattern
// 4.1 Prototype Design Pattern
// 4.1.1 Generate class
class Moster {
    // ...
};

class Ghost : public Monster {};
class Demon : public Monster {};
class Sorcerer : public Moster {};

// 4.1.2 Create spawner
class Spawner {
public :
    virtual ~spawner() {};
    virtual Monster* spawnMonster() = 0;
};

class GhostSpawner : public Spawner {
public :
    virtual Moster* spawnMonster() {
        return new Ghost();
    }
};

// ... more

// 4.1.3 Prototype pattern's point is that whitch object can spawn like itself.
//       So virtual method clone() is added in parent class Monster
class Moster {
public :
    virtual ~Moster() {}
    virtual Monster* clone() = 0;

    // ...
};

// 4.1.4 make clone()
class Ghost : public Monster {
public :
    Ghost(int health, int speed)
        : _health(health), _speed(speed) {}

    Monster* spawnMonster() { return _monster->clone(); }

private :
    Moster* _monster;   // prototype;
};

// 4.1.5 If you want to make ghost spawner, you deliver to spawner after creating ghost instance
Monster* ghostPrototype = new Ghost(15, 3);
Spawner* ghostSpawner = new Spawner(ghostPrototype);

// 4.2 Is this show well?

// 4.3 Spaw function
// 4.3.1 Let's create spawn function
Moster* spawnGhost() {
    return new Ghost();
}

// 4.3.2 Now Only one function pointer should be exist in spawner class
typedef Monster* (*SpawnCallback)();

class Spawner {
public :
    Spawner(SpawnCallback spawn) : _spawn(spawn) {}
    Monster* spawnMonster() { return _spawn(); }

private :
    SpawnCallback _spawn;
};

// 4.3.2 To spawn ghost object can make this
Spawner* ghostSpawner = new Spawner(spawnGhost);

// 4.4 Template
// 4.4.1 If you don't want to do hardcoding, Monster class can deliver as template type.
class Spawner {
public :
    virtual ~Spawner() {}
    virtual Monster* spawnMonster() = 0;
};

template <class T>
class SpawnerFor : public Spawner {
public :
    virtual Monster* spawnMonster() {
        return new T();
    }
};

// 4.4.2 Manual
Spawner* ghostSpawner = new SpawnerFor<Ghost>();


