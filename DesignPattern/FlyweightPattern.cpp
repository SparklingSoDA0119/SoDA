// 2. Flyweight Pattern
// 2.1 Trees
// 2.1.1
class Tree {
private :
    Mesh mesh;
    Texture bark;
    Texture leaves;
    Vector position;
    double height;
    double thickness;
    Color barTint;
    Color leafTint;
};

// 2.1.2 Most data in Tree class are not different each instances
//       Divine object half and these points can do modeling proposedly. 
//       Let's do gathering in to the new class what data that all tree use.
class TreeModel {
private :
    Mesh mesh;
    Texture bark;
    Texture leaves;
};

// 2.1.3 Let's left different state values each instances in Tree class
class Tree {
private :
    TreeModel* model;

    Vector position;
    double height;
    double thickness;
    Color harkTint;
    Color leafTink;
};

// 2.2 A few thousand instance

// 2.3 Flyweight Pattern
//     It is used when you want make more lower weight becasue object numbers is very much.

// 2.4 Topography information
