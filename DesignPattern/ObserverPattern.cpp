// 3. Observer Pattern
// 3.1 Accomplish
// 3.1.1 Observer pattern
void Physics::updateEntity(Entity& entity) {
    bool wasOnSurface = entity.isOnSurface();
    entity.accelerate(GRAVITY);
    entity.update();

    if (wasOnSurface && !entity.isOnSurface()) {
        notify(entity, EVENT_START_FALL);
    }
}

// 3.2 Operation method
// 3.3 Opserver
// 3.3.1
class Observer {
public :
    virtual ~Observer() {}
    virtual void onNotify(const Entity& entity, Event event) = 0;
};

// 3.3.2 Any classes can become a observer, if you make Observer interface
class Achivement : public Observer {
public :
    virtual void onNotify(const Entity& entity, Event event) {
        switch (event) {
            case EVENT_ENTITY_FELL :
                if (entity.isHero() && heroIsOnBridge_) {
                    unlock(ACHIVEMENT_FELL_OFF_BRIDGE);
                }
                break;
                // ... more
        }
    }

private :
    void unlock(Achivement achivement) {
    }
    bool _heroIsOnBridge;
};

// 3.4 Subject
// 3.4.1 Observered object call nofify method.
//       Subject has 2 missions. One of the mission is having a observer list what it is waiting notify.
class Subject {
private :
    Observer* observer* _observers[MAX_OBSERVERS];
    int _numObservers;
};

// 3.4.2 It is important thing is that API have opened "public" state so that the observer lists can be changed outside
class Subject {
public :
    void addObserver(Observer* observer) {
        // ... add
    }

    void removeObserver(Observer* observer) {
        // ... add;
    }

    // ... more
};

// 3.4.3 Observers can guarantee that observers are handled independently if many observers can be resistered
class Subject {
protected :
    void notify(const Entity& entity, Event event) {
        for (int i = 0; i < _numObservers; ++i) {
            _observers[i]->onNotify(entity, event);
        }
    }

    // ... more
};

// 3.5 Physical observing
// 3.5.1 물리 엔진에 훅을 걸어 알림을 보낼 수 있게 하는 일과 업적 시스템에서 알림을 받을 수 있도록 스스로를 등록하게 하는 일
class Physics : public subject {
public :
    void updateEntity(Entity& entity);
};


