from concurrent import futures
import grpc
import grpc.fr_pb2_grpc as fr_grpc
import logging
import fr

class FRServer(fr_grpc.fr_serviceServicer):
    def __init__(self):
        super().__init__()
        self.fr = fr()

    def destroy(self):
        self.is_running = False

    def init_fr_request(self, request, context):
         return super().init_fr_request(request, context)
    
    def cancel_fr_request(self, request, context):
        return super().cancel_fr_request(request, context)

def serve():
        server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
        fr_grpc.add_fr_serviceServicer_to_server(FRServer(), server)

        server.add_insecure_port('[::]:6003')
        server.start()
        server.wait_for_termination()
            
def main(args=None):
    logging.basicConfig()
    serve()

if __name__ == '__main__':
    main()