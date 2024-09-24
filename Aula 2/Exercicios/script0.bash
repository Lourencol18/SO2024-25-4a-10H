#l/usr/bin/bash
echo "Hello word!" 
s="Hello word 2!"
echo $s
pwd

cmd="ls"
$cmd

echo $1
echo "What´s your name?"
read name
echo "Hello $name"

echo "Comando a executar?"
read comando
$comando

echo "Diretório a criar"
read dir
mkdir temp
mkdir temp/$dir
ls temp

echo "Novo nome do diretório"
read novodir
mv temp/$dir temp/$novodir
ls temp