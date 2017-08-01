/*************************************************************************/
/*  arvr_server.h                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#ifndef ARVR_SERVER_H
#define ARVR_SERVER_H

#include "os/thread_safe.h"
#include "reference.h"
#include "rid.h"
#include "variant.h"

class ARVRInterface;
class ARVRPositionalTracker;

/**
	@author Bastiaan Olij <mux213@gmail.com>

	The ARVR server is a singleton object that gives access to the various
	objects and SDKs that are available on the system.
	Because there can be multiple SDKs active this is exposed as an array
	and our ARVR server object acts as a pass through
	Also each positioning tracker is accessible from here.

	I've added some additional info into this header file that should move
	into the documention, I will do so when we're close to accepting this PR
	or as a separate PR once this has been merged into the master branch.
**/

class ARVRServer : public Object {
	GDCLASS(ARVRServer, Object);
	_THREAD_SAFE_CLASS_

public:
	enum TrackerType {
		TRACKER_CONTROLLER = 0x01, /* tracks a controller */
		TRACKER_BASESTATION = 0x02, /* tracks location of a base station */
		TRACKER_ANCHOR = 0x04, /* tracks an anchor point, used in AR to track a real live location */
		TRACKER_UNKNOWN = 0x80, /* unknown tracker */

		TRACKER_ANY_KNOWN = 0x7f, /* all except unknown */
		TRACKER_ANY = 0xff /* used by get_connected_trackers to return all types */
	};

private:
	Vector<Ref<ARVRInterface> > interfaces;
	Vector<ARVRPositionalTracker *> trackers;

	Ref<ARVRInterface> primary_interface; /* we'll identify one interface as primary, this will be used by our viewports */

	real_t world_scale; /* scale by which we multiply our tracker positions */
	Transform world_origin; /* our world origin point, maps a location in our virtual world to the origin point in our real world tracking volume */
	Transform reference_frame; /* our reference frame */

	bool is_tracker_id_in_use_for_type(TrackerType p_tracker_type, int p_tracker_id) const;

protected:
	static ARVRServer *singleton;

	static void _bind_methods();

public:
	static ARVRServer *get_singleton();

	/* 
		World scale allows you to specify a scale factor that is applied to all positioning vectors in our VR world in essence scaling up, or scaling down the world.
		For stereoscopic rendering specifically this is very important to give an accurate sense of scale.
		Add controllers into the mix and an accurate mapping of real world movement to percieved virtual movement becomes very important.

		Most VR platforms, and our assumption, is that 1 unit in our virtual world equates to 1 meter in the real mode.
		This scale basically effects the unit size relationship to real world size.

		I may remove access to this property in GDScript in favour of exposing it on the ARVROrigin node
	*/
	real_t get_world_scale() const;
	void set_world_scale(real_t p_world_scale);

	/*
		The world maps the 0,0,0 coordinate of our real world coordinate system for our tracking volume to a location in our
		virtual world. It is this origin point that should be moved when the player is moved through the world by controller
		actions be it straffing, teleporting, etc. Movement of the player by moving through the physical space is always tracked
		in relation to this point.

		Note that the ARVROrigin spatial node in your scene automatically updates this property and it should be used instead of 
		direct access to this property and it therefor is not available in GDScript

		Note: this should not be used in AR and should be ignored by an AR based interface as it would throw what you're looking at in the real world
		and in the virtual world out of sync
	*/
	Transform get_world_origin() const;
	void set_world_origin(const Transform p_origin);

	/*
		Requesting a reference frame results in a matrix being calculated that ensures the HMD is positioned to 0,0,0 facing 0,0,-1 (need to verify this direction)
		in the virtual world. 

		Note: this should not be used in AR and should be ignored by an AR based interface as it would throw what you're looking at in the real world
		and in the virtual world out of sync
	*/
	Transform get_reference_frame() const;
	void request_reference_frame(bool p_ignore_tilt, bool p_keep_height);

	/*
		Interfaces are objects that 'glue' Godot to an AR or VR SDK such as the Oculus SDK, OpenVR, OpenHMD, etc. 
	*/
	void add_interface(const Ref<ARVRInterface> &p_interface);
	void remove_interface(const Ref<ARVRInterface> &p_interface);
	int get_interface_count() const;
	Ref<ARVRInterface> get_interface(int p_index) const;
	Ref<ARVRInterface> find_interface(const String &p_name) const;

	/*
		note, more then one interface can technically be active, especially on mobile, but only one interface is used for
		rendering. This interface identifies itself by calling set_primary_interface when it is initialized
	*/
	Ref<ARVRInterface> get_primary_interface() const;
	void set_primary_interface(const Ref<ARVRInterface> &p_primary_interface);
	void clear_primary_interface_if(const Ref<ARVRInterface> &p_primary_interface); /* this is automatically called if an interface destructs */

	/*
		Our trackers are objects that expose the orientation and position of physical devices such as controller, anchor points, etc. 
		They are created and managed by our active AR/VR interfaces.

		Note that for trackers that 
	*/
	int get_free_tracker_id_for_type(TrackerType p_tracker_type);
	void add_tracker(ARVRPositionalTracker *p_tracker);
	void remove_tracker(ARVRPositionalTracker *p_tracker);
	int get_tracker_count() const;
	ARVRPositionalTracker *get_tracker(int p_index) const;
	ARVRPositionalTracker *find_by_type_and_id(TrackerType p_tracker_type, int p_tracker_id) const;

	ARVRServer();
	~ARVRServer();
};

#define ARVR ARVRServer

VARIANT_ENUM_CAST(ARVRServer::TrackerType);

#endif