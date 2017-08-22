/******************************************************************
  1. Command Pattern
******************************************************************/
void InputHandler::handlerInput()
{
    if (isPressed(BUTTON_X)) {
        jump();
    }
    else if (isPressed(BUTTON_Y)) {
        fireGun();
    }
    else if (isPressed(BUTTON_A)) {
        swapWeapon();
    }
    else if (isPressed(BUTTON_B)) {
        kneel();
    }  
}
// 1.1 Changing input key
// 1.1.1 Define common parents class
class Command {
public :
    virtual ~Command() {}
    virtual void execute() = 0;
};

// 1.1.2 Make subclass
class JumpCommand : public Command {
public :
    virtual void execute() { jump(); }
};

class FireCommand : public Command {
public :
    virtual void execute() { fireGun(); }
};

// .. more

// 1.1.3 Save command class pointer depending on each buttuns.
class InutHandler {
public :
    void handleInput();

private :
    Command* buttonX_;
    Command* buttonY_;
    Command* buttonA_;
    Command* buttonB_;
};

// 1.1.4 Input process entrust to here.
void InputHandler::handleInput()
{
    if (isPressed(BUTTON_X)) {
        buttonX_->excute();
    }
    else if (isPressed(BUTTON_Y)) {
        buttonY_->excute();
    }
    else if (isPressed(BUTTON_A)) {
        buttonA_->excute();
    }
    else if (isPressed(BUTTON_B)) {
        buttonB_->excute();
    }  
}

// 1.2 Directing to actor
// 1.2.1 Object to contorl delivery at outside
class Command {
public :
    virtual ~Command() {}
    virtual void excute(GameActor& actor) = 0;
}

// 1.2.2 When called excute(), the inheritanced class cant wanted actor's method.
class JumpCommand : public Command {
public :
    virtual void excute(GameActor& actor) {
        actor.jump();
    }
};
//.... more

// 1.2.3 
void InputHandler::handleInput()
{
    if (isPressed(BUTTON_X)) {
        return buttonX_;
    }
    else if (isPressed(BUTTON_Y)) {
        return buttonY_;
    }
    else if (isPressed(BUTTON_A)) {
        return buttonA_;
    }
    else if (isPressed(BUTTON_B)) {
        return buttonB_;
    }  

    return NULL;
}

// 1.2.4 Code is needed that applying command object to GameActor
Command* command = inputHandler.handleInput();
if (command) {
    command->execute(actor);
}

// 1.3 Action cancellation & reaction
// 1.3.1 Think about moving unit.
class MoveUniCommand : public Command {
public :
    MoveUnitCommand(Unit* unit, int x, int y) : unit_(unit), x_(x), y_(y) {}

    virtual void execute() { unit_->moveTo(x_, y_); }

private :
    Unit* unit_;
    int x_, y_;
};

// 1.3.2 Input handler code generate command instance every time when user select moving.
Command* handleInput()
{
    Unit* unit = getSelectedUnit();

    if (isPressed(BUTTON_UP)) {
        int destY = unit->getY() - 1;
        return MoveUnitCommand(unit, unit->getx(), destY);
    }

    // ... more

    return NULL;
}

// 1.3.3 Defining inocent virtual function "undo()" in Command class for cancellation of command
class Command {
public :
    virtual ~Command() {}
    virtual void execute() = 0;
    virtual void undo() = 0;
};

// 1.3.4 Game state change in undo()
class MoveUnitCommand : public Command {
public :
    MoveUnitCommand(Unit* unit, int x, int y) 
        : _unit(unit), _x(x), _y(y), _xBefore(0), _yBefore(0) {}

    virtual void execute() {
        _xBefore = _unit->getX();
        _yBefore = _unit->getY();
        _unit->moveTo(_x, _y);
    }

    virtual void undo() {
        _unit->moveTo(_xBefore, _yBefore);
    }

private :
    Unit* _unit;
    int _x, _y;
    int _xBefore, _yBefore;
};



