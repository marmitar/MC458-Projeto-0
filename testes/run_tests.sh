#!/bin/sh

if [ -f mobile.c ]; then
    gcc -pedantic -Wall -O3 -o mobile *.c
elif [ -f mobile.cpp ]; then
    g++ -pedantic -Wall -O3 -o mobile *.cpp
else
    echo "Arquivo de código não encontrado! Verifique o nome do arquivo."
    exit 0
fi

if [ ! -d minhas_saidas ]; then
    mkdir minhas_saidas
fi

for f in entradas/*.in;
do
    bname=`basename $f`
    ofile=`echo $bname | sed "s/\.[^\.]*$//"`
    ./mobile < $f > minhas_saidas/$ofile.out

    if diff saidas/$ofile.out minhas_saidas/$ofile.out > /dev/null; then
	echo "$bname: Correta."
    else
  	echo "$bname: Incorreta."
    fi
done
