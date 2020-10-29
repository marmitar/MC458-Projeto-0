#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>



#define attribute(...) \
	__attribute__((__VA_ARGS__))


typedef struct mobile mobile_t;

#define SUBMOBILE 0ULL



typedef struct objeto {
	union {
		size_t peso;
		mobile_t *filho;
	} dado;
	enum { PESO, FILHO } tipo;
} objeto_t;


struct mobile {
	objeto_t Pe, Pd;
	size_t De, Dd;
};


attribute(const)
objeto_t objeto_nao_inicializado(void) {
	return (objeto_t) {
		.dado = {.peso = 0},
		.tipo = PESO
	};
}

attribute(const)
mobile_t mobile_nao_inicializado(void) {
	return (mobile_t) {
		.Pe = objeto_nao_inicializado(),
		.Pd = objeto_nao_inicializado(),
		.De = 0, .Dd = 0
	};
}

attribute(const)
mobile_t *mobile_filho(objeto_t obj) {
	if (obj.tipo == FILHO) {
		return obj.dado.filho;
	} else {
		return NULL;
	}
}

void free_mobile(mobile_t *mob) {
	int error = errno;

	mobile_t *filho = mobile_filho(mob->Pe);
	if (filho != NULL) {
		free_mobile(filho);
	}
	filho = mobile_filho(mob->Pd);
	if (filho != NULL) {
		free_mobile(filho);
	}
	free(mob);

	errno = error;
}


#define OK 	0
#define ERR -1
#define ENTINV 0x1234

attribute(format(scanf, 2, 3))
int cscanf(unsigned expect, const char *restrict fmt, ...) {
	va_list args;
	va_start(args, fmt);

	int rv = vscanf(fmt, args);
	if (rv < 0) {
		return rv;
	}
	if (rv < expect) {
		errno = ENTINV;
		return ENTINV;
	}
	return OK;
}

mobile_t *ler_mobile(void);

int ler_filho(objeto_t *obj, size_t peso) {
	if (peso == SUBMOBILE) {
		mobile_t *filho = ler_mobile();
		if (filho == NULL) {
			return ERR;
		}
		obj->dado.filho = filho;
		obj->tipo = FILHO;

		return OK;
	} else {
		obj->dado.peso = peso;
		obj->tipo = PESO;

		return OK;
	}
}

mobile_t *ler_mobile(void) {
	size_t Pe, De, Pd, Dd;
	int rv = cscanf(4, "%zu %zu %zu %zu\n", &Pe, &De, &Pd, &Dd);
	if (rv != OK) {
		return NULL;
	}

	mobile_t *mob = malloc(sizeof(mobile_t));
	if (mob == NULL) {
		return NULL;
	}
	*mob = mobile_nao_inicializado();
	mob->De = De;
	mob->Dd = Dd;

	rv = ler_filho(&mob->Pe, Pe);
	if (rv != OK) {
		free_mobile(mob);
		return NULL;
	}

	rv = ler_filho(&mob->Pd, Pd);
	if (rv != OK) {
		free_mobile(mob);
		return NULL;
	}

	return mob;
}

typedef struct equilibrio {
	size_t peso;
	bool equilibrio;
} equilibrio_t;


equilibrio_t mobile_estavel(mobile_t *mob);

equilibrio_t objeto_estavel(objeto_t objeto) {
	mobile_t *filho = mobile_filho(objeto);
	if (filho != NULL) {
		return mobile_estavel(filho);
	} else {
		return (equilibrio_t) {
			.peso = objeto.dado.peso,
			.equilibrio = true
		};
	}
}

equilibrio_t mobile_estavel(mobile_t *mob) {
	equilibrio_t esq, dir;

	esq = objeto_estavel(mob->Pe);
	dir = objeto_estavel(mob->Pd);

	bool estavel = (esq.peso * mob->De == dir.peso * mob->Dd);

	return (equilibrio_t) {
		.peso = esq.peso + dir.peso,
		.equilibrio = esq.equilibrio && dir.equilibrio && estavel
	};
}

void show_error(const char *prog) {
	if (errno == ENTINV) {
		(void) fprintf(stderr, "%s: entrada inválida\n", prog);
	} else {
		perror(prog);
	}
}


/**
 * Imprime "S" se emBalanco for maior que 0, e "N" caso contrario.
 */
void imprimeSaida(bool emBalanco) {
	(void) printf("%s\n", (emBalanco ? "S" : "N"));
}

int main(int argc, const char *argv[]) {
	assert(argc > 0);
	const char *prog = argv[0];

	size_t _len;
	int rv = cscanf(1, "%zu\n", &_len);
	if (rv != OK) {
		show_error(prog);
		return EXIT_FAILURE;
	}

	mobile_t *mobile = ler_mobile();
	if (mobile == NULL) {
		show_error(prog);
		return EXIT_FAILURE;
	}

	equilibrio_t estabilidade = mobile_estavel(mobile);
	imprimeSaida(estabilidade.equilibrio);

	free_mobile(mobile);
	return EXIT_SUCCESS;
}
