// 6. State Pattern
// 6.1 Making Game
// 6.1.1 Super Mario game
void Heroine::handleInput(Input input)
{
    if (input == PRESS_B) {
        yVelocity = JUMP_VELOCITY;
        setGrapthics(IMAGE_JUMP);
    }
}

// 6.1.2 check Jumping
void Heroine::handleInput(Input input)
{
    if (input == PRESS_B) {
        if (!_isJumping) {
            _isJumping = true;
        }
    }
}

// 6.1.3 Added function
void Heroine::handleInput(Input input)
{
    if (input == PRESS_B) {
    }
    else if (input == PRESS_DOWN) {
        if (!_isJumping) {
            setGraphics(IMAGE_DUCK);
        }
    }
    else if (input == RELEASE_DOWN) {
        setGraphics(IMAGE_STAND);
    }
}

// 6.1.4 It needs more flag vairables
void Heroine::handleInput(Input input)
{
    if (input == PRESS_B) {
        if (!_isJumping && !_isDucking) {
            // Jump
        }
    }
    else if (input == PRESS_DOWN) {
        if (!_isJumping) {
            _isDucking = true;
            setGraphics(IMAGE_DUCK);
        }
    }
    else if (input == RELEASE_DOWN) {
        if (_isDucking) {
            _isDucking = false;
            setGraphics(IMAGE_STAND);
        }
    }
}

// 6.2 FSM (Finite State Machine)
//     The conditions that can be obtained are limited
//     It can that have to be only one state at once.
//     'Input' or 'Event' are delivered to machine
//     Metastasis means that state change next state depending on input

// 6.3 enumeration type and multi-selection statement
// 6.3.1 FSM state can define as enumeration
enum State {
    STATE_STANDING,
    STATE_JUMPING,
    STATE_DUCKING,
    STATE_DIVING,
};

// 6.3.2 rouse

// 6.4 State Pattern
// 6.4.1 State Interface
class HeroineState {
public :
    virtual ~HeroineState() {}
    virtual void handleInput(Heroine& heroine, Input input) {}
    virtual void update(Heroine& heroine) {}
};

// 6.4.2 Making each state class
class DuckingStae : public HeroineState {
public :
    DuckingState() : _chargeTime(0) {}

    virtual void handleInput(Heroine& heroine, Input input) {
        if (input == RELEASE_DOWN) {
            heroine.setGraphincs(IMAGE_STAND);
        }
    }

    virtual void update(Heroine& heroine) {
        _chargeTime++;
        if (_chargeTime > MAX_CHANGE) {
            heroine.superBomb();
        }
    }

private :
    int _chargeTime;
};

// 6.4.3 Entrusting operation to state
class Heroine {
public :
    virtual void handleInput(Input input) {
        _state->handleInput(*this, input);
    }
    virtual void update() { _state->update(*this); }

private :
    HeroineState* _state;
};


