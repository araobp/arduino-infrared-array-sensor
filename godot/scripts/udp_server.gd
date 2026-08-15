extends Node3D

const PORT = 4242

var server : UDPServer = UDPServer.new()
var connected_peers: Array[PacketPeerUDP] = []

func _ready() -> void:
	var error = server.listen(PORT)
	if error == OK:
		print("UDP Server listening on port: ", PORT)
	else:
		print("Failed to start UDP Server. Error: ", error)

func _process(_delta: float) -> void:
	# Critical: Poll the socket every frame to check for new traffic
	server.poll()
	
	# Check for incoming new connections
	if server.is_connection_available():
		var new_peer: PacketPeerUDP = server.take_connection()
		print("New connection: %s:%d" % [new_peer.get_packet_ip(), new_peer.get_packet_port()])
		connected_peers.append(new_peer)
		
	# Process incoming data
	_handle_peer_data()

func _handle_peer_data() -> void:
	# Loop backward for safe array modification if peers need removal
	for i in range(connected_peers.size() - 1, -1, -1):
		var peer = connected_peers[i]
		
		# Process all packets available in the queue for this peer
		while peer.get_available_packet_count() > 0:
			var raw_packet: PackedByteArray = peer.get_packet()
			var message: String = raw_packet.get_string_from_utf8()
			
			print("[%s] Received: %s" % [peer.get_packet_ip(), message])
			
			if message == "swipe right":
				$Earth.rotate_earth("clockwise")
			elif message == "swipe left":
				$Earth.rotate_earth("counter_clockwise")
