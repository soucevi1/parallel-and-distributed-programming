# Paralelní a distribuované programování (semestrální práce)
Najděte pokrytí mřížky R dlaždicemi ve tvaru písmene I kromě zakázaných políček uvedených v množině D s maximální cenou. 

## Vstupní data:
* m,n = rozměry obdélníkové mřížky R[1..m,1..n], skládající se z m x n políček.
* 2 < i1 = počet políček tvořících dlaždice tvaru písmene I a délky i1 (= tvaru I1).
* i2 = počet políček tvořících dlaždice tvaru písmene I a délky i2 (= tvaru I2), i1 < i2 .
* 0 < c1 = cena bezkolizního umístění dlaždice tvaru I1.
* c2 = cena bezkolizního umístění dlaždice tvaru I2, c1 < c2.
* cn < 0 = cena (penalizace) políčka nepokrytého žádnou dlaždicí.
* k < m*n = počet zakázaných políček v mřížce R.
* D[1..k] = pole souřadnic k zakázaných políček rozmístěných v mřížce R.

## Úkol:
Najděte pokrytí mřížky R dlaždicemi ve tvaru písmene I kromě zakázaných políček uvedených v množině D s maximální cenou.
Cena pokrytí, ve kterém zbylo q nepokrytých nezakázaných políček, je

**c1 * počet dlaždic tvaru I1 + c2 * počet dlaždic tvaru I2 + cn * q .**

Dlaždice tvaru I1 nebo I2 lze do pokrytí pokládat vodorovně nebo svisle.

## Výstup algoritmu:
* Popis pokrytí mřížky R.
Např. m x n-maticí identifikátorů, kde každá dlaždice I je jednoznačně určena skupinou políček se stejným ID >=1, nepokrytá políčka jsou prázdná a zakázaná políčka jsou reprezentovaná znakem 'Z'.
* Výpis ceny, počtu a souřadnic nepokrytých políček. 
