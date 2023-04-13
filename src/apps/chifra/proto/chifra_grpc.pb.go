// Code generated by protoc-gen-go-grpc. DO NOT EDIT.
// versions:
// - protoc-gen-go-grpc v1.2.0
// - protoc             v3.21.11
// source: chifra.proto

package proto

import (
	context "context"
	grpc "google.golang.org/grpc"
	codes "google.golang.org/grpc/codes"
	status "google.golang.org/grpc/status"
)

// This is a compile-time assertion to ensure that this generated file
// is compatible with the grpc package it is being compiled against.
// Requires gRPC-Go v1.32.0 or later.
const _ = grpc.SupportPackageIsVersion7

// NamesClient is the client API for Names service.
//
// For semantics around ctx use and closing/ending streaming RPCs, please refer to https://pkg.go.dev/google.golang.org/grpc/?tab=doc#ClientConn.NewStream.
type NamesClient interface {
	// Search
	Search(ctx context.Context, in *SearchRequest, opts ...grpc.CallOption) (*SearchResponse, error)
	SearchStream(ctx context.Context, in *SearchRequest, opts ...grpc.CallOption) (Names_SearchStreamClient, error)
	// CRUD
	Create(ctx context.Context, in *CreateRequest, opts ...grpc.CallOption) (*CRUDResponse, error)
	Update(ctx context.Context, in *CreateRequest, opts ...grpc.CallOption) (*CRUDResponse, error)
	Delete(ctx context.Context, in *DeleteRequest, opts ...grpc.CallOption) (*CRUDResponse, error)
	Undelete(ctx context.Context, in *DeleteRequest, opts ...grpc.CallOption) (*CRUDResponse, error)
	Remove(ctx context.Context, in *DeleteRequest, opts ...grpc.CallOption) (*CRUDResponse, error)
}

type namesClient struct {
	cc grpc.ClientConnInterface
}

func NewNamesClient(cc grpc.ClientConnInterface) NamesClient {
	return &namesClient{cc}
}

func (c *namesClient) Search(ctx context.Context, in *SearchRequest, opts ...grpc.CallOption) (*SearchResponse, error) {
	out := new(SearchResponse)
	err := c.cc.Invoke(ctx, "/Names/Search", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *namesClient) SearchStream(ctx context.Context, in *SearchRequest, opts ...grpc.CallOption) (Names_SearchStreamClient, error) {
	stream, err := c.cc.NewStream(ctx, &Names_ServiceDesc.Streams[0], "/Names/SearchStream", opts...)
	if err != nil {
		return nil, err
	}
	x := &namesSearchStreamClient{stream}
	if err := x.ClientStream.SendMsg(in); err != nil {
		return nil, err
	}
	if err := x.ClientStream.CloseSend(); err != nil {
		return nil, err
	}
	return x, nil
}

type Names_SearchStreamClient interface {
	Recv() (*Name, error)
	grpc.ClientStream
}

type namesSearchStreamClient struct {
	grpc.ClientStream
}

func (x *namesSearchStreamClient) Recv() (*Name, error) {
	m := new(Name)
	if err := x.ClientStream.RecvMsg(m); err != nil {
		return nil, err
	}
	return m, nil
}

func (c *namesClient) Create(ctx context.Context, in *CreateRequest, opts ...grpc.CallOption) (*CRUDResponse, error) {
	out := new(CRUDResponse)
	err := c.cc.Invoke(ctx, "/Names/Create", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *namesClient) Update(ctx context.Context, in *CreateRequest, opts ...grpc.CallOption) (*CRUDResponse, error) {
	out := new(CRUDResponse)
	err := c.cc.Invoke(ctx, "/Names/Update", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *namesClient) Delete(ctx context.Context, in *DeleteRequest, opts ...grpc.CallOption) (*CRUDResponse, error) {
	out := new(CRUDResponse)
	err := c.cc.Invoke(ctx, "/Names/Delete", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *namesClient) Undelete(ctx context.Context, in *DeleteRequest, opts ...grpc.CallOption) (*CRUDResponse, error) {
	out := new(CRUDResponse)
	err := c.cc.Invoke(ctx, "/Names/Undelete", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

func (c *namesClient) Remove(ctx context.Context, in *DeleteRequest, opts ...grpc.CallOption) (*CRUDResponse, error) {
	out := new(CRUDResponse)
	err := c.cc.Invoke(ctx, "/Names/Remove", in, out, opts...)
	if err != nil {
		return nil, err
	}
	return out, nil
}

// NamesServer is the server API for Names service.
// All implementations must embed UnimplementedNamesServer
// for forward compatibility
type NamesServer interface {
	// Search
	Search(context.Context, *SearchRequest) (*SearchResponse, error)
	SearchStream(*SearchRequest, Names_SearchStreamServer) error
	// CRUD
	Create(context.Context, *CreateRequest) (*CRUDResponse, error)
	Update(context.Context, *CreateRequest) (*CRUDResponse, error)
	Delete(context.Context, *DeleteRequest) (*CRUDResponse, error)
	Undelete(context.Context, *DeleteRequest) (*CRUDResponse, error)
	Remove(context.Context, *DeleteRequest) (*CRUDResponse, error)
	mustEmbedUnimplementedNamesServer()
}

// UnimplementedNamesServer must be embedded to have forward compatible implementations.
type UnimplementedNamesServer struct {
}

func (UnimplementedNamesServer) Search(context.Context, *SearchRequest) (*SearchResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method Search not implemented")
}
func (UnimplementedNamesServer) SearchStream(*SearchRequest, Names_SearchStreamServer) error {
	return status.Errorf(codes.Unimplemented, "method SearchStream not implemented")
}
func (UnimplementedNamesServer) Create(context.Context, *CreateRequest) (*CRUDResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method Create not implemented")
}
func (UnimplementedNamesServer) Update(context.Context, *CreateRequest) (*CRUDResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method Update not implemented")
}
func (UnimplementedNamesServer) Delete(context.Context, *DeleteRequest) (*CRUDResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method Delete not implemented")
}
func (UnimplementedNamesServer) Undelete(context.Context, *DeleteRequest) (*CRUDResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method Undelete not implemented")
}
func (UnimplementedNamesServer) Remove(context.Context, *DeleteRequest) (*CRUDResponse, error) {
	return nil, status.Errorf(codes.Unimplemented, "method Remove not implemented")
}
func (UnimplementedNamesServer) mustEmbedUnimplementedNamesServer() {}

// UnsafeNamesServer may be embedded to opt out of forward compatibility for this service.
// Use of this interface is not recommended, as added methods to NamesServer will
// result in compilation errors.
type UnsafeNamesServer interface {
	mustEmbedUnimplementedNamesServer()
}

func RegisterNamesServer(s grpc.ServiceRegistrar, srv NamesServer) {
	s.RegisterService(&Names_ServiceDesc, srv)
}

func _Names_Search_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(SearchRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(NamesServer).Search(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/Names/Search",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(NamesServer).Search(ctx, req.(*SearchRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Names_SearchStream_Handler(srv interface{}, stream grpc.ServerStream) error {
	m := new(SearchRequest)
	if err := stream.RecvMsg(m); err != nil {
		return err
	}
	return srv.(NamesServer).SearchStream(m, &namesSearchStreamServer{stream})
}

type Names_SearchStreamServer interface {
	Send(*Name) error
	grpc.ServerStream
}

type namesSearchStreamServer struct {
	grpc.ServerStream
}

func (x *namesSearchStreamServer) Send(m *Name) error {
	return x.ServerStream.SendMsg(m)
}

func _Names_Create_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(CreateRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(NamesServer).Create(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/Names/Create",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(NamesServer).Create(ctx, req.(*CreateRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Names_Update_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(CreateRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(NamesServer).Update(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/Names/Update",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(NamesServer).Update(ctx, req.(*CreateRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Names_Delete_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(DeleteRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(NamesServer).Delete(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/Names/Delete",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(NamesServer).Delete(ctx, req.(*DeleteRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Names_Undelete_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(DeleteRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(NamesServer).Undelete(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/Names/Undelete",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(NamesServer).Undelete(ctx, req.(*DeleteRequest))
	}
	return interceptor(ctx, in, info, handler)
}

func _Names_Remove_Handler(srv interface{}, ctx context.Context, dec func(interface{}) error, interceptor grpc.UnaryServerInterceptor) (interface{}, error) {
	in := new(DeleteRequest)
	if err := dec(in); err != nil {
		return nil, err
	}
	if interceptor == nil {
		return srv.(NamesServer).Remove(ctx, in)
	}
	info := &grpc.UnaryServerInfo{
		Server:     srv,
		FullMethod: "/Names/Remove",
	}
	handler := func(ctx context.Context, req interface{}) (interface{}, error) {
		return srv.(NamesServer).Remove(ctx, req.(*DeleteRequest))
	}
	return interceptor(ctx, in, info, handler)
}

// Names_ServiceDesc is the grpc.ServiceDesc for Names service.
// It's only intended for direct use with grpc.RegisterService,
// and not to be introspected or modified (even as a copy)
var Names_ServiceDesc = grpc.ServiceDesc{
	ServiceName: "Names",
	HandlerType: (*NamesServer)(nil),
	Methods: []grpc.MethodDesc{
		{
			MethodName: "Search",
			Handler:    _Names_Search_Handler,
		},
		{
			MethodName: "Create",
			Handler:    _Names_Create_Handler,
		},
		{
			MethodName: "Update",
			Handler:    _Names_Update_Handler,
		},
		{
			MethodName: "Delete",
			Handler:    _Names_Delete_Handler,
		},
		{
			MethodName: "Undelete",
			Handler:    _Names_Undelete_Handler,
		},
		{
			MethodName: "Remove",
			Handler:    _Names_Remove_Handler,
		},
	},
	Streams: []grpc.StreamDesc{
		{
			StreamName:    "SearchStream",
			Handler:       _Names_SearchStream_Handler,
			ServerStreams: true,
		},
	},
	Metadata: "chifra.proto",
}
