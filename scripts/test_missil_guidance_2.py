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
MAX_VELOCITY = 80.0  # Vitesse maximale autorisée en m/s

# PID constants
Kp = 0.5  # Gain proportionnel
Ki = 0.1  # Gain intégral
Kd = 0.1  # Gain dérivé

# Initialisation
mass = 10.0  # Masse du missile en kg
thrust = 90.0  # Poussée du missile en Newtons

# Classes et fonctions
class Vector3D:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z
    
    def norm(self):
        return math.sqrt(self.x**2 + self.y**2 + self.z**2)
    
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
    drag_magnitude = 0.5 * AIR_DENSITY * speed**2 * DRAG_COEFFICIENT * CROSS_SECTIONAL_AREA
    return velocity.scale(-drag_magnitude / speed)

def calculate_lift(velocity):
    speed = velocity.norm()
    lift_magnitude = 0.5 * AIR_DENSITY * speed**2 * LIFT_COEFFICIENT * CROSS_SECTIONAL_AREA
    # lift_magnitude = mass * GRAVITY
    return Vector3D(0.0, lift_magnitude, 0.0)  # Supposons que la portance compense la gravité sur l'axe Y

def update_target_position(target, time):
    target.x += 10 * math.sin(0.001 * time)
    target.y += -10 * math.cos(0.001 * time)
    target.z -= 10 * TIME_STEP  # Déplacement constant vers l'avant
    return target.limit(MAX_VELOCITY*20.0)

position = Vector3D(500.0, 1000.0, 1000.0)  # Position initiale
velocity = Vector3D(0.0, 0.0, -22.0)  # Vitesse initiale
target = Vector3D(1000.0, 0.0, -1000.0)  # Position initiale de la cible

# Initialisation du PID
previous_error = Vector3D(0.0, 0.0, 0.0)
integral = Vector3D(0.0, 0.0, 0.0)

# Liste pour stocker les positions pour le graphique
positions = []
target_positions = []

time = 0.0  # Temps initial
time_max = 15000
while position.sub(target).norm() > 1.0 and time_max > 0 and position.y > 0:  # Tolérance pour atteindre la cible
    positions.append((position.x, position.y, position.z))
    target_positions.append((target.x, target.y, target.z))
    
    # target = update_target_position(target, time)  # Mettre à jour la position de la cible
    
    gravity_force = Vector3D(0.0, -mass * GRAVITY, 0.0)  # Gravité sur l'axe Y
    drag_force = calculate_drag(velocity)
    lift_force = calculate_lift(velocity)  # Portance pour compenser la gravité
    
    # Calcul de l'erreur de guidage
    error = target.sub(position).limit(MAX_DISTANCE)
    integral = integral.add(error.scale(TIME_STEP))
    derivative = error.sub(previous_error).scale(1 / TIME_STEP)
    previous_error = error
    
    # Calcul de l'accélération de guidage PID
    guidance_acceleration = error #.scale(Kp).add(integral.scale(Ki)).add(derivative.scale(Kd))
    
    
    
    # Force de poussée ajustée en fonction de la direction de l'erreur
    thrust_force = guidance_acceleration.scale(thrust / guidance_acceleration.norm())
    
    total_force = gravity_force.add(drag_force).add(thrust_force).add(lift_force)
    acceleration = total_force.scale(1.0 / mass).limit(MAX_DISTANCE)
    
    velocity = velocity.add(acceleration.scale(TIME_STEP)).limit(MAX_VELOCITY)  # Limiter la vitesse maximale
    position = position.add(velocity.scale(TIME_STEP)).limit(MAX_DISTANCE)

    time += TIME_STEP
    print(f"Position: ({position.x}, {position.y}, {position.z})")
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

ax.plot(z_vals, x_vals, y_vals, label='Trajectoire du missile')
ax.plot(target_z_vals, target_x_vals, target_y_vals, label='Trajectoire de la cible', linestyle='dashed')

ax.scatter( target.z, target.x, target.y,  color='red', label='Cible')
ax.scatter(position.z, position.x, position.y, color='blue', label='Missile')   
ax.scatter(positions[0][2], positions[0][0], positions[0][1], color='green', label='Missile start')

for i in range(100,len(positions) - 1, len(positions) // 10):
    ax.scatter(positions[i][2], positions[i][0], positions[i][1], color='blue')

for i in range(100,len(positions) - 1, len(positions) // 10):
    ax.scatter(target_positions[i][2], target_positions[i][0], target_positions[i][1], color='orange')

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
