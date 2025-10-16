from concurrent import futures
from typing import Optional
import grpc
import logging
import random
import fr_pb2
import fr_pb2_grpc
import fr_worker

class FRServer(fr_pb2_grpc.fr_serviceServicer):
    class RemoteSession:
        def __init__(self, sid: str) -> None:
            self.sid_: str = sid
            self.active_: bool = True
            self.name_: str = ""
            self.language_: str = ""

        def deactivate(self) -> None:
            self.active_ = False

        def set_name(self, name: str) -> None:
            self.name_ = name

        def set_language(self, language: str) -> None:
            self.language_ = language

    def __init__(self) -> None:
        super().__init__()
        self.logger : logging.Logger = logging.getLogger("FRServer")
        self.logger.setLevel(logging.DEBUG)

        self.worker_: fr_worker.FRWorker = fr_worker.FRWorker()

        self.sessions_: dict[str, FRServer.RemoteSession] = (
            {}
        )  # HashMap from session_id (string) to RemoteSession

        # Placeholder data for testing
        self.test_names: list[str] = [
            "Alice",
            "Bob",
            "Charlie",
            "Diana",
            "Eve",
            "Frank",
            "Grace",
            "Henry",
        ]
        self.test_languages: list[str] = ["English", "Japanese"]

        self.logger.debug("FR server started")

    def destroy(self) -> None:
        self.is_running = False

    def init_fr_request(
        self, request: fr_pb2.fr_request_message, context: grpc.ServicerContext
    ) -> fr_pb2.fr_response_message:
        """
        Placeholder function for face recognition initialization.
        Returns a random identity for testing purposes.
        """
        session_id: str = request.session_id
        self.logger.debug(f"Received init_fr_request for session {session_id}")

        # Create or update session
        if session_id not in self.sessions_:
            self.sessions_[session_id] = FRServer.RemoteSession(session_id)

        # session: FRServer.RemoteSession = self.sessions_[session_id]

        id, name, language = self.worker_.faceid()

        # Create identity message
        identity: fr_pb2.identity = fr_pb2.identity(
            id=id, name=name, language=language
        )

        self.logger.debug(f"returning status for session {session_id}")

        # Return response
        return fr_pb2.fr_response_message(
            session_id=session_id, success=True, result=identity
        )

    def cancel_fr_request(
        self, request: fr_pb2.fr_request_message, context: grpc.ServicerContext
    ) -> fr_pb2.fr_response_message:
        """
        Placeholder function for cancelling face recognition.
        Deactivates the session and returns the last known identity.
        """
        session_id: str = request.session_id
        self.logger.debug(f"Received cancel_fr_request for session {session_id}")

        if session_id in self.sessions_:
            session: FRServer.RemoteSession = self.sessions_[session_id]
            session.deactivate()

            # Return empty identity
            identity: fr_pb2.identity = fr_pb2.identity(id="", name="", language="")

            self.logger.debug(
                f"cancel_fr_request for session {session_id}: Session cancelled"
            )

            return fr_pb2.fr_response_message(
                session_id=session_id, success=True, result=identity
            )
        else:
            # Session not found
            self.logger.warning(
                f"cancel_fr_request for session {session_id}: Session not found"
            )

            # Return empty identity
            identity: fr_pb2.identity = fr_pb2.identity(id="", name="", language="")

            return fr_pb2.fr_response_message(
                session_id=session_id, success=False, result=identity
            )


def serve() -> None:
    server: grpc.Server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    fr_pb2_grpc.add_fr_serviceServicer_to_server(FRServer(), server)

    server.add_insecure_port("[::]:6003")
    server.start()
    server.wait_for_termination()


def main(args: Optional[list[str]] = None) -> None:
    logging.basicConfig()
    serve()


if __name__ == "__main__":
    main()
