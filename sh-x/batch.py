import multiprocessing
from cycle import cycle_and_export


if __name__ == "__main__":
    num_pages = int(
        input("How many pages of QR codes would you like to generate (note: 20 codes per page)? ")
    )
    
    for page_num in range(1, num_pages + 1):
        print(f"Generating page {page_num}... ", end="", flush=True)
        cycle_and_export(page_num)    
        print("DONE")
    
    print(f"Generated a total of {num_pages} pages of QR codes.")
