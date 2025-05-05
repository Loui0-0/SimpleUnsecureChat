
# Projet Systemes d'exploitation



## Fonctionnalités  
- Messagerie en temps réel : Les messages sont diffusés à tous les clients du même canal instantanément.  
- Identification des utilisateurs : Chaque message affiche le nom d'utilisateur de l'expéditeur.  
- Système de canaux : Les utilisateurs peuvent créer ou changer de canal via des commandes.  
- Composition de message : Appuyez sur Ctrl+C pour interrompre l'affichage et rédiger un message sans distraction.  

## Utilisation  

### Compilation  
1. Compiler le serveur :  
```
   gcc -g -Wall -I include -o server src/server.c src/socketUtils.c src/chat.c src/packet.c src/debug.c -lpthread
```
2. Compiler le client :  
```
   gcc -g -Wall -I include -o client src/client.c src/socketUtils.c src/chat.c src/packet.c src/debug.c src/renderer.c -lpthread 
```
### Exécution du serveur  
1. Démarrer le serveur : 
``` 
   ./server
```  
   Le serveur affichera le port d'écoute (exemple : 12345).  

### Exécution du client  
1. Démarrer le client : 
``` 
   ./client  
```
2. Saisir votre nom d'utilisateur lorsque demandé.  
3. Saisir le numéro de port du serveur (affiché par le serveur).  

### Commandes  
- Changer de canal : ```/selectChannel <ID> ou /sc <ID> ``` 
- Créer un canal :``` /createChannel <ID> ``` 
- Exemple : ```/createChannel 2``` crée un nouveau canal avec l'ID 2.  

## Remarques  
- Les messages sont visibles uniquement par les utilisateurs du même canal.  
- Par défaut, tous les utilisateurs rejoignent le canal 0 lors de la connexion.  

## Auteur
Louis de Domingo 