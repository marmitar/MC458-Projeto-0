#include <cstdlib>
#include <iostream>
#include <memory>


using std::pair;
using std::move;



class Objeto {
    public:
        typedef pair<bool, size_t> resultado;

        virtual ~Objeto() {};

        virtual resultado teste() const = 0;

        bool emEquilibrio() const {
            return std::get<0>(teste());
        }

        size_t peso() const {
            return std::get<1>(teste());
        }
};


class Peso: public Objeto {
    private:
        size_t valor;

    public:
        Peso(size_t peso): valor(peso) {
            if (peso == 0) {
                throw std::domain_error("peso inválido.");
            }
        }

        resultado teste() const override {
            return pair(emEquilibrio(), peso());
        }

        bool emEquilibrio() const {
            return true;
        }

        size_t peso() const {
            return valor;
        }
};

class Mobile: public Objeto {
    private:
        typedef std::unique_ptr<Objeto> Filho;

        Filho E, D;
        size_t De, Dd;

        Mobile(Filho E, Filho D, size_t De, size_t Dd): E(move(E)), D(move(D)), De(De), Dd(Dd) {}

        static
        Filho filho(size_t peso, std::istream &in = std::cin) {
            try {
                return std::make_unique<Peso>(peso);
            } catch (std::domain_error&) {
                return std::make_unique<Mobile>(read(in));
            }
        }

    public:
        resultado teste() const override {
            size_t Pe, Pd, Qe, Qd;
            std::tie(Qe, Pe) = E->teste();
            std::tie(Qd, Pd) = D->teste();

            bool Q = (Pe * De == Pd * Dd);

            return pair(Qe && Qd && Q, Pe + Pd);
        }

        static
        Mobile read(std::istream &in = std::cin) {
            size_t Pe, Pd, De, Dd;
            in >> Pe >> De >> Pd >> Dd;

            auto E = filho(Pe, in);
            auto D = filho(Pd, in);

            return Mobile(move(E), move(D), De, Dd);
        }
};

int main(void) {
    size_t _k;
    std::cin >> _k;

    Mobile mobile = Mobile::read();

    if (mobile.emEquilibrio()) {
        std::cout << "S" << std::endl;
    } else {
        std::cout << "N" << std::endl;
    }
    return EXIT_SUCCESS;
}
