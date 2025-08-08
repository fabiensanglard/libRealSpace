import math
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Constantes
GRAVITY = 9.81  # Accélération due à la gravité (m/s^2)
AIR_DENSITY = 1.225  # Densité de l'air (kg/m^3)
DRAG_COEFFICIENT = 0.47  # Coefficient de traînée
CROSS_SECTIONAL_AREA = 0.1  # Section transversale (m^2)
TIME_STEP = 0.01  # Pas de temps pour l'intégration (s)
PROXIMITY_THRESHOLD = 10.0  # Distance en dessous de laquelle la vitesse est réduite
LIFT_COEFFICIENT = 0.5  # Coefficient de portance
MAX_DISTANCE = 1e6  # Distance maximale pour éviter les valeurs trop grandes

# PID constants
Kp = 0.1  # Gain proportionnel
Ki = 0.001  # Gain intégral
Kd = 0.1  # Gain dérivé

# Classes et fonctions
class Vector3D:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z
    
    def norm(self):
        return math.sqrt(self.x**2 + self.y**2 + self.z**2)
    def normalize(self):
        norm = self.norm()
        return Vector3D(self.x / norm, self.y / norm, self.z / norm)
    def add(self, other):
        return Vector3D(self.x + other.x, self.y + other.y, self.z + other.z)
    
    def sub(self, other):
        return Vector3D(self.x - other.x, self.y - other.y, self.z - other.z)
    
    def scale(self, scale):
        return Vector3D(self.x * scale, self.y * scale, self.z * scale)
    
    def limit(self, max_value):
        if self.norm() > max_value:
            scale_factor = max_value / self.norm()
            return self.scale(scale_factor)
        return self

def calculate_drag(velocity):
    speed = velocity.norm()
    drag_magnitude = 0.5 * AIR_DENSITY *  DRAG_COEFFICIENT * CROSS_SECTIONAL_AREA
    return velocity.scale(-drag_magnitude)

def calculate_lift(velocity, orientation): 
    speed = velocity.norm() 
    lift_magnitude = 0.5 * AIR_DENSITY * LIFT_COEFFICIENT * CROSS_SECTIONAL_AREA 
    lift_direction = Vector3D(-orientation.y, orientation.x, 0) # perpendiculaire à l'orientation 
    return lift_direction.scale(lift_magnitude)

def update_target_position(target, time):
    target.x += 10 * math.sin(0.001 * time)
    #target.y += .05 * math.cos(0.001 * time)
    target.z -= 10 * TIME_STEP  # Déplacement constant vers l'avant
    return target

# Initialisation
mass = 10.0  # Masse du missile en kg
thrust = 3# Poussée du missile en Newtons
position = Vector3D(500.0, 1000.0, 1000.0)  # Position initiale
velocity = Vector3D(0.0, 0.0, 0.0)  # Vitesse initiale
target = Vector3D(1000.0, 1000.0, -1000.0)  # Position initiale de la cible
MAX_SPEED = 2.0 # Vitesse maximale du missile
# Initialisation du PID
previous_error = Vector3D(0.0, 0.0, 0.0)
integral = Vector3D(0.0, 0.0, 0.0)

# Liste pour stocker les positions pour le graphique
positions = []
target_positions = []

time = 0.0  # Temps initial
time_max = 15000
# il faut implémenter une vitesse maximale pour eviter les valeurs trop grandes

while position.sub(target).norm() > 1.0 and time_max > 0:  # Tolérance pour atteindre la cible
    positions.append((position.x, position.y, position.z))
    target_positions.append((target.x, target.y, target.z))
    
    target = update_target_position(target, time)  # Mettre à jour la position de la cible
    
    gravity_force = Vector3D(0.0, -mass * GRAVITY, 0.0)  # Gravité sur l'axe Y

    # Calcul de l'erreur de guidage
    error = target.sub(position).normalize()
    integral = integral.add(error)
    derivative = error.sub(previous_error)
    previous_error = error
    
    # Calcul de l'accélération de guidage PID
    guidance_acceleration = error
    
    # Force de poussée ajustée en fonction de la direction de l'erreur
    thrust_force = guidance_acceleration.scale(thrust*mass)
    lift_force = Vector3D(0, mass*GRAVITY, 0)
    total_force = thrust_force.add(gravity_force).add(lift_force)
    acceleration = total_force.scale(1.0 / mass)
    
    velocity = velocity.add(acceleration.scale(TIME_STEP))
    # calc vitesse max pour au bout d'un moment avoir une vitesse terminal permettant le guidage, ni trop vite
    # ni trop lent
    velocity = velocity.limit(1)
    position = position.add(velocity)
    print(position.z, position.x, position.y, velocity.norm())
    time += TIME_STEP
    time_max -= 1

# Tracer la trajectoire
positions.append((position.x, position.y, position.z))
target_positions.append((target.x, target.y, target.z))

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Extraire les coordonnées
x_vals = [pos[0] for pos in positions]
y_vals = [pos[1] for pos in positions]
z_vals = [pos[2] for pos in positions]

target_x_vals = [pos[0] for pos in target_positions]
target_y_vals = [pos[1] for pos in target_positions]
target_z_vals = [pos[2] for pos in target_positions]

ax.plot(z_vals, x_vals, y_vals,  label='Trajectoire du missile')
ax.plot(target_z_vals, target_x_vals, target_y_vals, label='Trajectoire de la cible', linestyle='dashed')
ax.scatter( target.z, target.x, target.y,  color='red', label='Cible')
ax.scatter(position.z, position.x, position.y, color='blue', label='Missile')   
ax.scatter(positions[0][2], positions[0][0], positions[0][1], color='green', label='Missile start')

for i in range(100,len(positions) - 1, 100):
    ax.scatter(positions[i][2], positions[i][0], positions[i][1], color='blue')
# Ajouter des étiquettes et une légende
ax.set_xlabel('Z (arrière-avant)')
ax.set_ylabel('X (gauche-droite)')
ax.set_zlabel('Y (haut-bas)')
ax.legend()

plt.show()

# Calculer les angles d'azimut et d'élévation
direction = target.sub(position)
azimut = math.atan2(direction.x, -direction.z)  # Ajusté pour correspondre au nouveau repère
elevation = math.atan2(direction.y, math.sqrt(direction.x**2 + direction.z**2))

print("Le missile a atteint la cible!")
print(f"Azimut: {azimut} radians ({azimut * 180 / math.pi} degrés)")
print(f"Élévation: {elevation} radians ({elevation * 180 / math.pi} degrés)")
print(time_max, 's')