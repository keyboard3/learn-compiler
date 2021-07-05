#include "models.h"

class Machine {
    public:
        Context * context;
        Machine();
        void process(string code);
};