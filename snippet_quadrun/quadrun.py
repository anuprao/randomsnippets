#!/usr/bin/env python3
# -*- coding: utf-8 -*-

bl_info = {
	"name" : "Fix Non-Planar",
	"category" : "Mesh"
	}

import os
import math
import json

import bpy
import bpy_extras

import bmesh

from mathutils import geometry

####################################################################################################

# From https://blenderartists.org/forum/showthread.php?266051-compare-two-vectors
def compareVector(v1, v2):
    return round(v1[0],4) == round(v2[0],4) and \
        round(v1[1],4) == round(v2[1],4) and \
        round(v1[2],4) == round(v2[2],4)
        
####################################################################################################

class FindNonPlanar(bpy.types.Operator):
	bl_idname = "object.find_nonplanar"
	bl_label = "Find Non Planar"
	bl_options = {'REGISTER', 'UNDO'}
	
	@classmethod
	def poll(cls, context):
		return context.active_object is not None
		
	def execute(self, context):
		
		scene = context.scene
		cursor = scene.cursor_location
		
		obj = bpy.context.edit_object
		me = obj.data		
		
		bm = bmesh.from_edit_mesh(me)
		
		############################################################################################
		
		bpy.ops.mesh.select_all(action='DESELECT')
		
		arrNonPlanarFaces = []
		
		for f in bm.faces:
			#print("face",f)
			f.select = False
			
			#for v in f.verts:
			#	print("vert",v)
			
			verts = f.verts
			
			if 4 == len(verts):
				n0 = geometry.normal(verts[0].co, verts[1].co, verts[2].co)
				n1 = geometry.normal(verts[0].co, verts[1].co, verts[3].co)
				n2 = geometry.normal(verts[0].co, verts[2].co, verts[3].co)
				n3 = geometry.normal(verts[1].co, verts[2].co, verts[3].co)
				
				'''
				print(f.normal)
				print(n0)
				print(n1)
				print(n2)
				print(n3)
				
				print(compareVector(n0,f.normal))
				print(compareVector(n1,f.normal))
				print(compareVector(n2,f.normal))
				print(compareVector(n3,f.normal))
				'''
				
				bPlanar = False
				if	compareVector(n0,f.normal) and \
					compareVector(n1,f.normal) and \
					compareVector(n2,f.normal) and \
					compareVector(n3,f.normal) :
					
					bPlanar = True			
				
				if False == bPlanar :
					
					arrNonPlanarFaces.append(f)
					f.select = True
			
			print("")
		
		############################################################################################
		
		return {'FINISHED'}

class FixNonPlanarEdgeBias(bpy.types.Operator):
	bl_idname = "object.fix_nonplanar_edgebias"
	bl_label = "Fix Non Planar Edge Bias"
	bl_options = {'REGISTER', 'UNDO'}
	
	@classmethod
	def poll(cls, context):
		return context.active_object is not None
		
	def execute(self, context):
		
		scene = context.scene
		cursor = scene.cursor_location
		
		obj = bpy.context.edit_object
		me = obj.data		
		
		bm = bmesh.from_edit_mesh(me)
		
		print("\nFix Non Planar - Edge Bias")
		
		############################################################################################
		
		# Obtain face of interest FOI;  E1, E2 and one of the points EB exists on FOI
		
		FOI = None
		
		# simple way
		#print("Listing selected faces : ")
		for f in bm.faces:
			if f.select:
				#print(f.index)
				#print(f.edges)
				
				FOI = f
		
		#print("FOI", FOI)
		
		edges_in_selected_face = []
				
		for sampleEdge in FOI.edges:
			edges_in_selected_face.append(sampleEdge.index)
		
		#print("Edges in Selected Face", edges_in_selected_face)
		
		verts_in_selected_face = []
				
		for sampleVertex in FOI.verts:
			verts_in_selected_face.append(sampleVertex.index)
		
		#print("Vertices in Selected Face", verts_in_selected_face)
		
		'''
		# Another Way
		for geom in bm.select_history:
			if isinstance(geom, BMFace):
			print(geom.index)
		'''
				
		# Obtain bias edge EB
		
		EB = None
		
		#print("Listing selected edges : ")
		for e in bm.edges:
			if e.select:
				#print(e.index)
				if e.index not in edges_in_selected_face:
					EB = e
		
		#print("Bias Edge", EB)	
		
		arrVertsInBiasEdge = []
		for v in EB.verts:
			arrVertsInBiasEdge.append(v.index)
		
		#print("arrVertsInBiasEdge", arrVertsInBiasEdge)
		
		# Obtain bias pivot vertex VBP; it should be part of FOI and vertex from where EB originates
		
		#print("FOI.vertices", FOI.verts)
		
		VBP = None
		
		arrVertsInFOI = []
		for v in FOI.verts:
			
			#print(v)
			
			arrVertsInFOI.append(v.index)
			
			if v.index in arrVertsInBiasEdge:
				VBP = v.index
				
		#print("VBP", VBP)
		
		# Obtain other vertices as V1 and V2 which are connected to VBP
		
		sample_vs = []
		for sampleEdge in FOI.edges:
			
			vs = sampleEdge.verts[0].index
			ve = sampleEdge.verts[1].index
			
			if vs == VBP:
				sample_vs.append(ve)
			elif ve == VBP:
				sample_vs.append(vs)
				
		V1 = sample_vs[0]
		V2 = sample_vs[1]
		
		#print("V1", V1)
		#print("V2", V2)
		
		# Obtain common vertex VC; it should not be connected to VBP i.e, opposite to VBP
		verts_in_selected_face.remove(VBP)
		verts_in_selected_face.remove(V1)
		verts_in_selected_face.remove(V2)
		VC = verts_in_selected_face[0]
		
		#print("VC", VC)
		
		## optional:  Obtain intersecting edges E1 and E2; VC is one of the vertex of E1 and E2
			
		# Form a temporary face bm_FTmp with VC and EB
		
		VS_EB = arrVertsInBiasEdge[0]
		VE_EB = arrVertsInBiasEdge[1]
		
		bm_VBP = bm.verts[VBP]
		bm_VC = bm.verts[VC]
		bm_VS_EB = bm.verts[VS_EB]
		bm_VE_EB = bm.verts[VE_EB]
		
		bm_FTmp = bm.faces.new( [bm_VC, bm_VS_EB, bm_VE_EB] )
		
		arrNewEdges_FTmp = []
		for sampleEdge in bm_FTmp.edges:
			if EB != sampleEdge:
				arrNewEdges_FTmp.append(sampleEdge)
		
		print("bm_FTmp.normal", bm_FTmp.normal)
		
		# Form a temporary edge bm_ETmp with V1 and V2
		
		bm_ETmp = bm.edges.new( [bm.verts[V1], bm.verts[V2]] )
		
		bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
		
		# Find pt of intersection VI, between ETmp and bm_FTmp
		#print(type(bm.verts[V1].co), type(bm.verts[V2].co), type(bm.verts[VC].co), type(bm_FTmp.normal))
		plane_co = bm_FTmp.calc_center_median()
		plane_normal = bm_FTmp.normal
		ptVI = geometry.intersect_line_plane(bm.verts[V1].co, bm.verts[V2].co,plane_co, plane_normal)
		#print("ptVI", ptVI)
		
		bm_VI = bm.verts.new(ptVI)
		#print(bm_VI.index)
		
		# Form another temporary edge EI_Tmp between VC and VI
		bm_EDirTmp = bm.edges.new([bm_VC, bm_VI])
		
		# Obtain vertex VF at intersection of EI_Tmp and EB
		ptVF = geometry.intersect_line_line(bm_VC.co, bm_VI.co, bm_VS_EB.co, bm_VE_EB.co)
		if ptVF:
			ptVF = (ptVF[0] + ptVF[1])/2
			
			bm_VF = bm.verts.new(ptVF)
		
			# Move vertex VBP to VF; Merge, ALT+M with Last option
			bmesh.ops.pointmerge(bm, verts=[bm_VBP], merge_co=bm_VF.co)
		
		#Cleanup
		
		'''	
		ref: bmesh_operator_api.h

		enum {
			DEL_VERTS = 1,
			DEL_EDGES, #2
			DEL_ONLYFACES, #3
			DEL_EDGESFACES, #4
			DEL_FACES, #5
			DEL_ALL, #6
			DEL_ONLYTAGGED #7
		}
		'''

		# Delete all temporary faces; bm_FTmp
		bmesh.ops.delete(bm, geom=[bm_FTmp], context=3)
		bm_FTmp = None
		
		# Delete all temporary edges; ETmp, EI_Tmp
		bmesh.ops.delete(bm, geom=[bm_ETmp, bm_EDirTmp], context=2)
		bm_ETmp = None
		bm_EDirTmp = None
		
		bmesh.ops.delete(bm, geom=arrNewEdges_FTmp, context=2)
		arrNewEdges_FTmp = None
		
		# Delete all temporary vertices; VI, VF
		bmesh.ops.delete(bm, geom=[bm_VF], context=1)
		bm_VF = None
		
		## optional: Remove all doubles
		
		# Show the updates in the viewport
		# and recalculate n-gon tessellation.
		bmesh.update_edit_mesh(me, True)
				
		############################################################################################
		
		return {'FINISHED'}
		
####################################################################################################
def menu_func(self, context):
	
	self.layout.operator(FindNonPlanar.bl_idname)
	self.layout.operator(FixNonPlanarEdgeBias.bl_idname)

def register():
	
	#if menu_func in bpy.types.VIEW3D_MT_edit_mesh:
	#	bpy.types.VIEW3D_MT_edit_mesh.remove(menu_func)
		
	if hasattr(bpy.types, "find_nonplanar"):
		bpy.utils.unregister_class(FindNonPlanar)
	
	bpy.utils.register_class(FindNonPlanar)
	
	#
	
	if hasattr(bpy.types, "fix_nonplanar_edgebias"):
		bpy.utils.unregister_class(FixNonPlanarEdgeBias)
	
	bpy.utils.register_class(FixNonPlanarEdgeBias)	
	
	#
	
	bpy.types.VIEW3D_MT_edit_mesh.append(menu_func)
	
def unregister():
	bpy.utils.unregister_class(FindNonPlanar)
	bpy.utils.unregister_class(FixNonPlanarEdgeBias)

def main():
	register()
	
if "__main__" == __name__:
	main()
