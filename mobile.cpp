/**
 * Este arquivo pode ser usado como base para o seu codigo.
 * Preste especial atencao a funcao ja implementada "imprimeSaida". Esta função
 * deve ser usada para imprimir o resultado da execucao de cada algoritmo.
 */

#include <cstdlib>
#include <iostream>
#include <variant>
#include <memory>
#include <utility>


using std::unique_ptr;
using std::variant;
using std::pair;
using std::move;

class Mobile {
private:
	typedef variant<size_t, unique_ptr<Mobile>> peso_t;

	size_t De, Dd;
	peso_t Pe, Pd;

	static
	pair<size_t, bool> massa_estavel_filho(const peso_t &peso) {
		if (std::holds_alternative<size_t>(peso)) {
			auto massa = std::get<0>(peso);
			return pair(massa, true);

		} else {
			return std::get<1>(peso)->massa_estavel();
		}
	}

	pair<size_t, bool> massa_estavel() const {
		size_t Pe, Pd;
		bool Se, Sd;

		std::tie(Pe, Se) = massa_estavel_filho(this->Pe);
		std::tie(Pd, Sd) = massa_estavel_filho(this->Pd);

		bool S = Se && Sd && (Pe * De == Pd * Dd);
		return pair(Pe + Pd, S);
	}

public:
	Mobile(size_t Pe, size_t De, size_t Pd, size_t Dd):  De(De), Dd(Dd), Pe(Pe), Pd(Pd) {}
	Mobile(Mobile Pe, size_t De, size_t Pd, size_t Dd):  De(De), Dd(Dd), Pd(Pd) {
		unique_ptr<Mobile> ptr(new Mobile(move(Pe)));
		this->Pe = move(ptr);
	}
	Mobile(size_t Pe, size_t De, Mobile Pd, size_t Dd):  De(De), Dd(Dd), Pe(Pe) {
		unique_ptr<Mobile> ptr(new Mobile(move(Pd)));
		this->Pd = move(ptr);
	}
	Mobile(Mobile Pe, size_t De, Mobile Pd, size_t Dd):  De(De), Dd(Dd) {
		unique_ptr<Mobile> ptre(new Mobile(move(Pe)));
		this->Pe = move(ptre);
		unique_ptr<Mobile> ptrd(new Mobile(move(Pd)));
		this->Pd = move(ptrd);
	}

	size_t peso() const {
		return std::get<0>(massa_estavel());
	}

	bool estavel() const {
		return std::get<1>(massa_estavel());
	}

	friend std::ostream& operator<<(std::ostream& stream, const Mobile &data) {
		bool peso_esq = std::holds_alternative<size_t>(data.Pe);
		bool peso_dir = std::holds_alternative<size_t>(data.Pd);

		size_t Pe = peso_esq? std::get<0>(data.Pe) : 0;
		size_t Pd = peso_dir? std::get<0>(data.Pd) : 0;

		stream << Pe << ' ' << data.De << ' ' << Pd << ' ' << data.Dd << std::endl;

		if (!peso_esq) {
			stream << *std::get<1>(data.Pe);
		}
		if (!peso_dir) {
			stream << *std::get<1>(data.Pd);
		}
		return stream;
	}

	friend std::istream& operator>>(std::istream &stream, Mobile &data) {
		size_t Pe, De, Pd, Dd;

		stream >> Pe >> De >> Pd >> Dd;

		if (Pe != 0 && Pd != 0) {
			data = Mobile(Pe, De, Pd, Dd);
		} else if (Pd != 0) {
			Mobile MPe = read(stream);
			data = Mobile(move(MPe), De, Pd, Dd);
		} else if (Pe != 0) {
			Mobile MPd = read(stream);
			data = Mobile(Pe, De, move(MPd), Dd);
		} else {
			Mobile MPe = read(stream);
			Mobile MPd = read(stream);
			data = Mobile(move(MPe), De, move(MPd), Dd);
		}

		return stream;
	}

	static
	Mobile read(std::istream &in) {
		Mobile mob(0, 0, 0, 0);
		in >> mob;
		return mob;
	}
};

/**
 * Imprime "S" se emBalanco for verdadeiro, e "N" caso contrario.
 */
void imprimeSaida(bool emBalanco) {
	std::cout << (emBalanco ? "S" : "N") << std::endl;
}

int main(int argc, char const *argv[]) {
	size_t _k;
	std::cin >> _k;

	Mobile m = Mobile::read(std::cin);
	std::cout << m;

	imprimeSaida(m.estavel());

	return EXIT_SUCCESS;
}
