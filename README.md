# Instrucciones

## Compilar

Compilar lab2

```
make -f makefile1
```

## Test

Ejecutar los archivos de prueba
```
./lab2 -i visibilidades.csv -o propiedades.txt -d ancho -n ndiscos -h numerohebras -c chunk -b
```

100 Visibilidades
```
./lab2 -i test100.csv -o propiedades.txt -d 200 -n 2 -h 3 -c 30 -b
```

10000 Visibilidades
```
./lab2 -i test10000.csv -o propiedades.txt -d 100 -n 4 -h 5 -c 100 -b
```

## Limpiar

```
make -f makefile1 clean
```
