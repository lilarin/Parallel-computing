import random
from locust import HttpUser, task, between

class WebsiteUser(HttpUser):
    wait_time = between(5, 15)  # час очікування між запитами, від 5 до 15 секунд

    @task
    def index(self):
        self.client.get("/")

    @task
    def view_page(self):
        pages = ['/kemalia.html', '/fraklia.html', '/kliantys.html']
        page = self.client.get(pages[random.randint(0, len(pages)-1)])
