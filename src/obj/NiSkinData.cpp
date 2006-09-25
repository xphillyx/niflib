/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#include "../../include/obj/NiSkinData.h"
#include "../../include/obj/NiNode.h"
#include "../../include/gen/SkinData.h"
#include "../../include/gen/SkinWeight.h"
#include "../../include/obj/NiSkinPartition.h"
#include "../../include/obj/NiGeometry.h"
#include "../../include/obj/NiSkinInstance.h"
using namespace Niflib;

//Definition of TYPE constant
const Type NiSkinData::TYPE("NiSkinData", &NI_SKIN_DATA_PARENT::TypeConst() );

NiSkinData::NiSkinData() NI_SKIN_DATA_CONSTRUCT {}

NiSkinData::~NiSkinData() {}

void NiSkinData::Read( istream& in, list<uint> & link_stack, unsigned int version, unsigned int user_version ) {
	InternalRead( in, link_stack, version, user_version );
}

void NiSkinData::Write( ostream& out, map<NiObjectRef,uint> link_map, unsigned int version, unsigned int user_version ) const {
	InternalWrite( out, link_map, version, user_version );
}

string NiSkinData::asString( bool verbose ) const {
	return InternalAsString( verbose );
}

void NiSkinData::FixLinks( const map<unsigned,NiObjectRef> & objects, list<uint> & link_stack, unsigned int version, unsigned int user_version ) {
	InternalFixLinks( objects, link_stack, version, user_version );
}

list<NiObjectRef> NiSkinData::GetRefs() const {
	return InternalGetRefs();
}

const Type & NiSkinData::GetType() const {
	return TYPE;
};

uint NiSkinData::GetBoneCount() const {
	return uint( boneList.size() );
}

Matrix44 NiSkinData::GetBoneTransform( uint bone_index ) const {
	if ( bone_index > boneList.size() ) {
		throw runtime_error( "The specified bone index was larger than the number of bones in this NiSkinData." );
	}

	return Matrix44( boneList[bone_index].translation, boneList[bone_index].rotation, boneList[bone_index].scale );
}

vector<SkinWeight> NiSkinData::GetBoneWeights( uint bone_index ) const {
	if ( bone_index > boneList.size() ) {
		throw runtime_error( "The specified bone index was larger than the number of bones in this NiSkinData." );
	}

	return boneList[bone_index].vertexWeights;
}

void NiSkinData::SetBoneWeights( uint bone_index, const vector<SkinWeight> & n, Vector3 center, float radius ) {
	if ( bone_index > boneList.size() ) {
		throw runtime_error( "The specified bone index was larger than the number of bones in this NiSkinData." );
	}

	boneList[bone_index].vertexWeights = n;
   boneList[bone_index].boundingSphereOffset = center;
   boneList[bone_index].boundingSphereRadius = radius;
}

Matrix44 NiSkinData::GetOverallTransform() const {
	return Matrix44( translation, rotation, scale );
}

NiSkinData::NiSkinData( const Ref<NiGeometry> & owner ) NI_SKIN_DATA_CONSTRUCT {
	//Get skin instance
	NiSkinInstanceRef skinInst = owner->GetSkinInstance();

	if ( skinInst == NULL ) {
		throw runtime_error("Skin instance should have already been created.");
	}

	boneList.resize( skinInst->GetBoneCount() );
	vector< Ref<NiNode> > bone_nodes = skinInst->GetBones();
	
	//--Calculate Overall Offset--//

	//Get TriBasedGeom world transform
	Matrix44 owner_mat = owner->GetWorldTransform();

	//Get Skeleton root world transform
	Matrix44 sr_world = skinInst->GetSkeletonRoot()->GetWorldTransform();

	//Inverse owner NiGeometry matrix & multiply with skeleton root matrix
	Matrix44 overall_mat = (owner_mat * sr_world.Inverse()).Inverse();

	//Store result
	overall_mat.Decompose( translation, rotation, scale );

	//--Calculate Bone Offsets--//
	Matrix44 res_mat;
	Matrix44 bone_mat;
	for (uint i = 0; i < boneList.size(); ++i ) {
		//--Get Bone Bind Pose--//

		//Get bone world position
		bone_mat = bone_nodes[i]->GetWorldTransform();

		//Multiply NiGeometry matrix with inversed bone matrix
		res_mat = owner_mat * bone_mat.Inverse();

		//Store result
		res_mat.Decompose( boneList[i].translation, boneList[i].rotation, boneList[i].scale );
	}
}

Ref<NiSkinPartition> NiSkinData::GetSkinPartition() const {
   return skinPartition;
}
void NiSkinData::SetSkinPartition(Ref<NiSkinPartition> value) {
   skinPartition = value;
}