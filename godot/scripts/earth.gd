extends MeshInstance3D

@export var target_angle: float = PI
@export var turn_speed: float = 5.0

func _process(delta: float) -> void:
	# Smoothly interpolates the current Y rotation to the target angle
	rotation.y = lerp_angle(rotation.y, target_angle, turn_speed * delta)

## Increments or decrements the Earth's target Y-axis orientation by 60 degrees (PI/3 radians).
func rotate_earth(rotation_direction):
	if rotation_direction == "clockwise":
		target_angle += PI/3
	elif rotation_direction == "counter_clockwise":
		target_angle -= PI/3
