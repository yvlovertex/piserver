# 🖥️ Télécommande Distante pour Raspberry Pi (Dashboard IoT)

Ce projet permet de contrôler et de surveiller un Raspberry Pi à distance via une application graphique moderne développée en **C++ (Qt 6)** sur PC Windows, communiquant avec un serveur d'arrière-plan écrit en **Python 3** s'exécutant sur le Raspberry Pi. 

La connexion réseau sécurisée en dehors du réseau local est assurée de manière transparente via un maillage de réseau privé **Tailscale VPN**.

<img width="715" height="595" alt="app" src="https://github.com/user-attachments/assets/29975f0e-1ec6-4b70-922c-96cc338b5b28" />

---

## 🛠️ Architecture du Projet

Le projet repose sur une architecture Client/Serveur découplée :
1. **Le Client (PC Windows)** : Interface graphique en C++ pur utilisant `QTcpSocket` pour envoyer des requêtes textuelles et afficher les résultats dans une console intégrée.
2. **Le Serveur (Raspberry Pi)** : Script Python géré par un service système (`systemd`) qui écoute en permanence sur le port `12345`, exécute les commandes Linux natives ou interagit avec Docker, puis renvoie le flux textuel au client.

<img width="3586" height="1248" alt="diagram" src="https://github.com/user-attachments/assets/1fe1cc88-90fc-44db-979b-6382cbd93a92" />

---

## ⚙️ Fonctionnalités Validées

### 📊 Surveillance Système
* **Espace Disque** : Analyse l'espace de stockage disponible (`df -h`).
* **Mémoire RAM** : Affiche la mémoire vive libre et utilisée (`free -h`).
* **Température CPU** : Température du processeur du Pi en direct (`vcgencmd measure_temp`).
* **Charge CPU** : Calcule le pourcentage d'utilisation instantané du processeur.
* **Uptime** : Indique depuis combien de temps le système tourne sans interruption.

### 🐳 Gestion Docker & CouchDB
* **Liste Containers** : Liste le statut des conteneurs actifs (Nom, Statut, Ports).
* **Logs CouchDB** : Récupère les 20 dernières lignes de logs du conteneur `couchdb` pour le débogage.
* **Arrêter / Redémarrer CouchDB** : Permet de manager le cycle de vie du conteneur à distance.

### 🔌 Contrôle Énergie
* **Redémarrage (`Reboot`)** : Relance complètement le Raspberry Pi à distance.
* **Arrêt (`Shutdown`)** : Éteint proprement le micro-ordinateur avant déconnexion physique.

---

## 🐍 Le Code Serveur (`piserver.py`)

Ce script doit être hébergé sur le Raspberry Pi à l'emplacement `/home/overtex/piserver.py`.

```python
import socket
import subprocess

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('0.0.0.0', 12345))
server.listen(1)

print("=== Serveur Pi Multi-commandes en ligne sur le port 12345 ===")

while True:
    client_socket, address = server.accept()
    print(f"Client connecté : {address}")
    client_socket.send(b"READY\n")
    
    while True:
        try:
            data = client_socket.recv(1024)
            if not data:
                break
            
            commande = data.decode('utf-8').strip()
            print(f"Ordre exécuté : {commande}")
            
            if commande == "GET_DISK":
                res = subprocess.check_output(["df", "-h", "/"])
                client_socket.send(res)
                
            elif commande == "GET_RAM":
                res = subprocess.check_output(["free", "-h"])
                client_socket.send(b"--- Utilisation de la Memoire RAM ---\n" + res)
                
            elif commande == "GET_TEMP":
                res = subprocess.check_output(["vcgencmd", "measure_temp"])
                client_socket.send(b"--- Temperature du Processeur ---\n" + res)
                
            elif commande == "GET_CPU":
                cmd = "top -bn1 | grep 'Cpu(s)' | sed \"s/.*, *\\([0-9.]*\\)%* id.*/\\1/\" | awk '{print 100 - $1\"% utilises\"}'"
                res = subprocess.check_output(cmd, shell=True)
                client_socket.send(b"--- Charge CPU Actuelle ---\n" + res)
                
            elif commande == "GET_UPTIME":
                res = subprocess.check_output(["uptime", "-p"])
                client_socket.send(b"--- Temps d'activite du Pi ---\n" + res)
                
            elif commande == "DOCKER_PS":
                res = subprocess.check_output(["docker", "ps", "--format", "table {{.Names}}\t{{.Status}}\t{{.Ports}}"])
                client_socket.send(b"--- Liste des conteneurs Docker ---\n" + res)
                
            elif commande == "DOCKER_COUCH_LOGS":
                res = subprocess.check_output(["docker", "logs", "--tail", "20", "couchdb"])
                client_socket.send(b"--- 20 dernieres lignes de logs CouchDB ---\n" + res)
                
            elif commande == "DOCKER_COUCH_STOP":
                subprocess.call(["docker", "stop", "couchdb"])
                client_socket.send(b"Succes : Conteneur CouchDB arrete.\n")
                
            elif commande == "REBOOT_COUCHDB":
                subprocess.call(["docker", "restart", "couchdb"])
                client_socket.send(b"Succes : Redemarrage de CouchDB initie.\n")
                
            elif commande == "PI_REBOOT":
                client_socket.send(b"Attention : Ordre recu, le Raspberry Pi va redemarrer...\n")
                subprocess.Popen(["sudo", "reboot"])
                break
                
            elif commande == "PI_SHUTDOWN":
                client_socket.send(b"Attention : Ordre recu, le Raspberry Pi s'eteint...\n")
                subprocess.Popen(["sudo", "poweroff"])
                break
                
            else:
                client_socket.send(b"Commande inconnue.\n")
        except Exception as e:
            error_msg = f"Erreur lors de l'execution : {str(e)}\n"
            client_socket.send(error_msg.encode('utf-8'))
            break
            
    print("Client déconnecté.")



````
## 🚀 Déploiement et Persistance sur le Pi

Pour s'assurer que le script s'exécute en arrière-plan à l'infini (même après redémarrage ou fermeture du terminal SSH), il est configuré comme un service de l'OS via `systemd`.

### 1. Création du service
```bash
sudo nano /etc/systemd/system/piserver.service
```

### 2. Configuration interne
```ini
[Unit]
Description=Serveur de Telecommande Qt pour Raspberry Pi
After=network.target

[Service]
Type=simple
User=overtex
WorkingDirectory=/home/overtex
ExecStart=/usr/bin/python3 /home/overtex/piserver.py
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target*
````

### 3. Commandes de gestion du cycle de vie

```bash
# Recharger la configuration pour prendre en compte les modifications
sudo systemctl daemon-reload

# Activer au démarrage automatique du Raspberry Pi
sudo systemctl enable piserver.service

# Démarrer le serveur immédiatement en tâche de fond
sudo systemctl start piserver.service

# Vérifier le statut en temps réel (doit afficher "active (running)")
sudo systemctl status piserver.service
````

---

## 🌐 Configuration Réseau Distant (WAN)

Pour utiliser l'application partout (4G/Wi-Fi extérieur) sans ouvrir les ports de la Box Internet, le projet exploite Tailscale.

* Installe Tailscale sur le PC Windows et sur le Raspberry Pi.
* Connecte les deux machines sur le même compte/réseau privé Tailscale.
* Récupère l'adresse IPv4 virtuelle attribuée au Pi (ex: 100.98.213.31).
* Renseigne cette adresse directement dans l'application Qt pour bénéficier d'un chiffrement de bout en bout et d'une connectivité globale instantanée.
